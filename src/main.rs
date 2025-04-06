#![no_std]
#![no_main]

use embassy_executor::Spawner;
use embassy_futures::join::*;
use embassy_usb::class::cdc_acm::State;
use esp_hal::{clock::CpuClock, otg_fs::Usb, rng::Rng, timer::timg::TimerGroup};
use esp_println::println;
use esp_wifi::{esp_now::EspNow, init};

extern crate alloc;

mod services;

#[panic_handler]
fn panic(_: &core::panic::PanicInfo) -> ! {
    loop {
        println!("Panic!");
    }
}

#[esp_hal_embassy::main]
async fn main(_spawner: Spawner) {
    let peripherals = esp_hal::init(esp_hal::Config::default().with_cpu_clock(CpuClock::max()));

    esp_alloc::heap_allocator!(size: 72 * 1024);

    let timg0 = TimerGroup::new(peripherals.TIMG0);
    esp_hal_embassy::init(timg0.timer0);

    // ESP-NOW 配置
    let esp_wifi_ctrl = init(
        timg0.timer1,
        Rng::new(peripherals.RNG),
        peripherals.RADIO_CLK,
    )
    .unwrap();

    // 初始化 ESP-NOW
    let esp_now = EspNow::new(&esp_wifi_ctrl, peripherals.WIFI).unwrap();

    let mut config_descriptor = [0u8; 256];
    let mut bos_descriptor = [0u8; 256];
    let mut control_buf = [0u8; 64];
    let mut ep_out_buffer = [0u8; 1024];
    let mut state = State::new();

    // 初始化 USB 外设
    let usb = Usb::new(peripherals.USB0, peripherals.GPIO20, peripherals.GPIO19);
    let (class, mut device) = services::usb_serial::init(
        usb,
        &mut config_descriptor,
        &mut bos_descriptor,
        &mut control_buf,
        &mut ep_out_buffer,
        &mut state,
    );

    let (_control, mut sender, mut receiver) = esp_now.split();
    let (mut tx, mut rx) = class.split();

    // 测试用的异步函数
    let usb2esp_fn = async {
        loop {
            rx.wait_connection().await;
            println!("Connected: usb2esp");

            let mut buf = [0; 1024];
            loop {
                let n = rx.read(&mut buf).await;
                if let Err(_) = n {
                    println!("Disconnected: usb2esp");
                    break;
                }
                let n = n.unwrap();
                let _ = sender.send_async(&[0xFF; 6], &buf[..n]).await;
            }
        }
    };

    let esp2usb_fn = async {
        loop {
            tx.wait_connection().await;
            println!("Connected: esp2usb");

            loop {
                let r = receiver.receive_async().await;
                if let Err(_) = tx.write(r.data()).await {
                    println!("Disconnected: esp2usb");
                    break;
                }
            }
        }
    };

    join3(device.run(), usb2esp_fn, esp2usb_fn).await;
}
