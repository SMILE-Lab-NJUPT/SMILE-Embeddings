#![allow(dead_code)]
use embassy_futures::select::{select, Either};
use embassy_time::{Duration, Timer};
use embassy_usb::{
    class::cdc_acm::{CdcAcmClass, Receiver, Sender, State},
    driver::EndpointError,
    Builder, UsbDevice,
};
use esp_hal::otg_fs::{
    asynch::{Config, Driver},
    Usb,
};

pub struct UsbSerial<'d> {
    sender: UsbSender<'d>,
    receiver: UsbReceiver<'d>,
}

impl<'d> UsbSerial<'d> {
    pub fn new(
        builder: &mut Builder<'d, Driver<'d>>,
        state: &'d mut State<'d>,
        max_packet_size: u16,
    ) -> UsbSerial<'d> {
        let class = CdcAcmClass::new(builder, state, max_packet_size);
        let (sender, receiver) = class.split();
        UsbSerial {
            sender: UsbSender { sender },
            receiver: UsbReceiver { receiver },
        }
    }

    pub async fn wait_connection(&mut self) {
        self.receiver.wait_connection().await;
    }

    pub async fn write(&mut self, data: &[u8]) -> Result<(), EndpointError> {
        Ok(self.sender.write(data).await?)
    }

    pub async fn read(&mut self, buffer: &mut [u8]) -> Result<usize, EndpointError> {
        Ok(self.receiver.read(buffer).await?)
    }

    pub fn split(self) -> (UsbSender<'d>, UsbReceiver<'d>) {
        (self.sender, self.receiver)
    }
}

pub struct UsbSender<'d> {
    sender: Sender<'d, Driver<'d>>,
}

impl UsbSender<'_> {
    pub async fn wait_connection(&mut self) {
        self.sender.wait_connection().await;
    }

    pub async fn write(&mut self, data: &[u8]) -> Result<(), EndpointError> {
        let chunk_size = 64;
        for chunk in data.chunks(chunk_size) {
            self.sender.write_packet(chunk).await?;
        }
        Ok(())
    }
}

pub struct UsbReceiver<'d> {
    receiver: Receiver<'d, Driver<'d>>,
}

impl UsbReceiver<'_> {
    pub async fn wait_connection(&mut self) {
        self.receiver.wait_connection().await;
    }

    pub async fn read(&mut self, buffer: &mut [u8]) -> Result<usize, EndpointError> {
        let mut total_read = 0;
        while total_read < buffer.len() {
            match select(
                self.receiver.read_packet(&mut buffer[total_read..]),
                Timer::after(Duration::from_millis(100)),
            )
            .await
            {
                Either::First(read) => {
                    let read = read?;
                    if read == 0 {
                        break;
                    }
                    total_read += read;
                }
                Either::Second(_) => {
                    if total_read != 0 {
                        break;
                    }
                }
            }
        }
        Ok(total_read)
    }
}

pub fn init<'d>(
    usb: Usb<'d>,
    config_descriptor: &'d mut [u8; 256],
    bos_descriptor: &'d mut [u8; 256],
    control_buf: &'d mut [u8; 64],
    ep_out_buffer: &'d mut [u8; 1024],
    state: &'d mut State<'d>,
) -> (UsbSerial<'d>, UsbDevice<'d, Driver<'d>>) {
    // 创建异步 USB Driver
    let driver = Driver::new(usb, ep_out_buffer, Config::default());

    // USB Driver 配置文件
    let mut config = embassy_usb::Config::new(0x303A, 0x3001);
    config.manufacturer = Some("NJUPT");
    config.product = Some("IoT_gateway");
    config.serial_number = Some("1145141919810");
    config.device_class = 0xEF;
    config.device_sub_class = 0x02;
    config.device_protocol = 0x01;
    config.composite_with_iads = true;

    let mut builder = Builder::new(
        driver,
        config,
        config_descriptor,
        bos_descriptor,
        &mut [],
        control_buf,
    );

    // 获取 USB 模拟串口
    let usb_serial = UsbSerial::new(&mut builder, state, 64);

    // 创建 USB 设备
    let device = builder.build();

    (usb_serial, device)
}
