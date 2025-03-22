#![no_std]
#![no_main]
extern crate alloc;

use alloc::string::ToString;
use embassy_executor::Spawner;
use embassy_futures::join::join;
use embassy_usb::{
    class::cdc_acm::{CdcAcmClass, State},
    driver::EndpointError,
};
use esp_hal::{
    clock::CpuClock,
    otg_fs::{asynch::Driver, Usb},
    rng::Rng,
    timer::timg::TimerGroup,
};
use esp_println::println;
use esp_wifi::{esp_now::EspNow, init};
use crate::services::usb_serial::UsbSerial;

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
    let mut esp_now = EspNow::new(&esp_wifi_ctrl, peripherals.WIFI).unwrap();

    let mut config_descriptor = [0u8; 256];
    let mut bos_descriptor = [0u8; 256];
    let mut control_buf = [0u8; 64];
    let mut ep_out_buffer = [0u8; 1024];
    let mut state = State::new();

    // 初始化 USB 外设
    let usb = Usb::new(peripherals.USB0, peripherals.GPIO20, peripherals.GPIO19);
    let (mut class,mut device) = services::usb_serial::init(
        usb,
        &mut config_descriptor,
        &mut bos_descriptor,
        &mut control_buf,
        &mut ep_out_buffer,
        &mut state,
    );

    // 测试用的异步函数
    let echo_fut = async {
        loop {
            class.wait_connection().await;
            println!("Connected");
            let _ = usb2espnow(&mut class, &mut esp_now).await;
            println!("Disconnected");
        }
    };

    join(device.run(), echo_fut).await;
}

async fn usb2espnow<'a>(
    class: &mut UsbSerial<'a>,
    esp_now: &mut EspNow<'_>,
) -> Result<(), EndpointError> {
    let mut buf = [0; 1024];
    loop {
        let n = class.read(&mut buf).await?;
        let data = &buf[..n];
        class.write(data).await?;
        // class.write(n.to_string().as_bytes()).await?;
        // println!("{:?}", &data);
        let status = esp_now.send(&[0xFF; 6], data).unwrap().wait();
        // class
        //     .write_packet(format!("{:?}", status).as_bytes())
        //     .await?;
        // let _ = esp_now.send(&[0xFF; 6], r#"{"device_name":"light1","MAC":"","data":{"color":[16777215,8355711,4144959,2039583,986895,460551,197379,65793]}}"#.as_bytes()).unwrap().wait();
    }
}
