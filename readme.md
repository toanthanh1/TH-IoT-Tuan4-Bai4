# Empty C Example
This example project shows an empty configuration that can be used as a starting point to add components and functionality.



Dưới đây là những thông tin cần thiết để hiểu bài Lab 1: Giao tiếp UART, được trích từ tài liệu bạn đang xem:

**1. Mục tiêu:**

  * Xây dựng chương trình giao tiếp Universal Synchronous Receiver/Transmitter (UART) giữa EFR32xG21 (C code + Simplicity Studio 5) và Host PC (C/C++ code + Visual Studio/Visual Code/...).
  * Thiết bị thực hành: BRD4180B Radio Board (EFR32xG21 2.4 GHz 20 dBm) + Wireless Starter Kit Mainboard (BRD4001A).
  * Phần mềm: Simplicity Studio 5 + Simplicity SDK Suite v.2024.6.2.

**2. Giới thiệu Wireless Starter Kit (WSK):**

  * EFR32xG21 Wireless Gecko Wireless SoC là một radio board gắn trực tiếp vào Wireless Starter Kit (Wireless STK) Mainboard.
  * Mainboard hỗ trợ:
      * J-Link debugger: Lập trình và debug EFR32xG21 qua USB hoặc Ethernet.
      * Advanced Energy Monitor (AEM): Theo dõi dòng điện và điện thế thời gian thực.
      * Virtual COM (VCOM): Cung cấp kết nối nối tiếp (serial connection) qua USB hoặc Ethernet.
  * Mainboard còn hỗ trợ các cảm biến (Si7021: nhiệt độ và độ ẩm) và thiết bị ngoại vi (LCD, Button, LED, 20-pin EXP header).

**3. UART trong EFR32xG21 Wireless SoC:**

  * UART tồn tại trong module Universal Synchronous Asynchronous Receiver/Transmitter (USART).
  * USART là một module serial I/O linh động, hỗ trợ giao tiếp bất đồng bộ UART theo cơ chế full duplex, cũng như RS-485, SPI, MicroWire và 3-wire.

**4. Kết nối TX và RX của UART:**

  * Sử dụng hai dây TX (Transmit) và RX (Receive).
  * TX của module 1 nối với RX của module 2.
  * RX của module 1 nối với TX của module 2 (nối chéo nhau).

**5. Định dạng frame (khung dữ liệu) UART:**

  * **Start bit (S):** Kéo đường truyền TX xuống mức thấp trong một chu kỳ bit để báo hiệu bắt đầu truyền.
  * **Data bit:** 4 đến 16 bit dữ liệu (mặc định 8 bit).
  * **Parity bit (P):** Bit kiểm tra chẵn lẻ (nếu được sử dụng) (mặc định không có).
      * 00: No parity bit
      * 01: (Không xác định trong bảng)
      * 10: Even parity
      * 11: Odd parity
  * **Stop bit:** Kéo TX lên mức cao để báo hiệu kết thúc quá trình truyền và sẵn sàng cho khung tiếp theo (mặc định 1 bit).
      * 00: 0.5 bit
      * 01: 1 bit (Default)
      * 10: 1.5 bit
      * 11: 2 bit

**6. Clock generation và Baud rate:**

  * Công thức tính baud rate: `br = f_PCLK / (oversample * (1 + USARTn_CLKDIV/256))`
      * `f_PCLK`: Tần số của peripheral clock.
      * `oversample`: Định nghĩa bởi OVS (00: 16, 01: 8, 10: không xác định, 11: 4).
  * USARTn\_CLKDIV là giá trị 20 bit (15 bit phần nguyên, 5 bit phần thập phân).

**7. Xây dựng chương trình UART cho EFR32xG21 (Sử dụng Simplicity Studio 5):**

  * **Bước 1-3:** Mở Simplicity Studio 5, tìm kiếm "EFR32xG21", chọn BRD4180B Radio Board, và tạo "Empty C Project".
  * **Bước 4-5:** Đặt tên project và xem cấu trúc file.
  * **Bước 6-8:** Thêm thư viện USART vào project thông qua Project Configurator.

**8. Chương trình demo (Cấu hình UART trên EFR32xG21):**

  * **VCOM:** Giao tiếp UART qua VCOM (Virtual COM) được chọn làm giao thức giữa EFR32xG21 và Host PC. Board Controller sẽ chuyển đổi tín hiệu UART-to-USB.
  * **Cấu hình pin TX và RX:**
      * TX (UART0): PA5
      * RX (UART0): PA6
      * VCOM\_ENABLE: PD4 (kích hoạt ở mức cao để sử dụng UART qua VCOM)
      * `initGPIO()`: Cấu hình các chân này.
  * **Cấu hình UART0 (`initUSARTO()`):**
      * `baudrate = 115200`
      * `oversampling = usartOVS16`
      * `databits = usartDatabits8`
      * `parity = usartNoParity`
      * `stopbits = usartStopbits1`
      * Route chân TX và RX của USART0 đến board controller.
      * `USART_InitAsync(USART0, &init);`
  * **Cách sử dụng UART:**
      * Hàm RX: `uint8_t buffer = USART_Rx(USART0);`
      * Hàm TX: `USART_Tx(USART0, buffer);`
  * **Các bước thực hiện:**
      * Sao chép file `main.c` mẫu vào project.
      * Biên dịch chương trình.
      * Kết nối WSTK với Host PC.
      * Nạp chương trình.
      * Kiểm thử bằng MobaXterm: Cấu hình cổng Serial (COM6, Speed 115200, Data bits 8, Stop bits 1, Parity None). Dữ liệu gửi từ PC sẽ được EFR32xG21 phản hồi (echo) lại PC.

**9. Xây dựng chương trình UART cho Window PC (Sử dụng Visual Studio hoặc công cụ C tương ứng):**

  * **Xác định COM port:** Trong Device Manager, tìm "Jlink CDC UART Port" (ví dụ: COM6).
  * **Chương trình mẫu:** Cấu hình baudrate, data bit, stop bit, parity. Gửi 1 byte dữ liệu và chờ nhận phản hồi.
  * **Các hàm chính:**
      * `CreateFileA()`: Tạo serial port.
      * `SetCommTimeouts()`: Cấu hình timeout cho đọc/ghi.
      * `SetCommState()`: Cài đặt Baud rate, Data bits, Stop bits, Parity bit.
      * `WriteFile()`: Truyền dữ liệu.
      * `ReadFile()`: Nhận dữ liệu.
      * `CloseHandle()`: Đóng serial port.
  * **Kết quả:** Gửi thành công 0xab và nhận lại 0xab.

**10. Bài chuẩn bị:**

  * UART là gì? Các thành phần cơ bản của một khung truyền (frame) UART là gì?
  * Vẽ và giải thích cấu trúc của một frame UART bao gồm start bit, data bit, parity bit, stop bit.
  * Frame Error và Parity Error là gì? Mô tả tình huống có thể dẫn đến các lỗi này.
  * Cài đặt Simplicity Studio 5, MobaXterm, Visual Studio hoặc các công cụ lập trình C tương tự.

**11. Bài thực hành:**

  * **Bài 1:** Xây dựng chương trình điều khiển trạng thái LED0 và LED1 trên EFR32xG21 từ Host PC qua UART.
      * Yêu cầu: Bật/tắt từng LED riêng biệt hoặc cả hai cùng lúc.
      * Chi tiết về sử dụng LED (Phụ lục):
          * LED0: PD02 (trên mainboard và LED đỏ trên radio board).
          * LED1: PD03 (trên mainboard và LED xanh lá trên radio board).
          * Tất cả các đèn LED đều được kích hoạt ở mức cao.
          * Các hàm thay đổi trạng thái LED: `GPIO_PinOutToggle()`, `GPIO_PinOutSet()`.
  * **Bài 2:** Viết chương trình truyền thông điệp từ Host PC và xác định thời gian truyền thông điệp đó qua UART.
      * Yêu cầu: Host PC gửi một thông điệp bất kỳ, EFR32xG21 nhận, xử lý và gửi lại phản hồi chứa tổng thời gian truyền tải thông điệp.
      * Lưu ý: Thời gian truyền tải thực tế bị ảnh hưởng bởi baud rate, số bit dữ liệu, số bit stop và parity.
      * Cấu hình chương trình demo: `f_PCLK` của EFR32xG21 là 40 MHz.
      * Thử nghiệm với các cấu hình baud rate, data bit, stop bit, parity khác nhau và điều chỉnh OVS của EFR32xG21.



Dưới đây là nội dung phần Phụ lục (BUTTON và LED) dưới dạng văn bản:

**Phụ lục**

**BUTTON và LED**

Bộ kit có tổng cộng bốn đèn LED được điều khiển bởi hai chân GPIO trên EFR32. Hai đèn LED màu vàng, được đánh dấu là LED0 và LED1, nằm trên Mainboard. Một đèn LED đỏ xanh lá được tích hợp trên Radio board \[1\].

Các đèn LED được kết nối như sau:

  * PD02 điều khiển LED0 trên bo mạch chính và đèn LED đỏ trên Radio board.
  * PD03 điều khiển LED1 trên bo mạch chính và đèn LED xanh lá trên Radio board.

Tất cả các đèn LED đều được kích hoạt ở mức cao.

**Chương trình mẫu:**

**Định Port & Pin của Button & LED**

``` c
#define BSP_GPIO_LEDS
#define BSP_GPIO_LED0_PORT gpioPortD
#define BSP_GPIO_LED0_PIN 2
#define BSP_GPIO_LED1_PORT gpioPortD
#define BSP_GPIO_LED1_PIN 3
#define BSP_GPIO_PB0_PORT gpioPortB
#define BSP_GPIO_PB0_PIN 0
#define BSP_GPIO_PB1_PORT gpioPortB
#define BSP_GPIO_PB1_PIN 1

```

**Hàm khởi tạo mode và interrupt của Button & LED**

``` c
void initLED_BUTTON(){
  // Enable GPIO clock
  CMU_ClockEnable(cmuClock_GPIO, true);

  // Configure PB0 and PB1 as input with glitch filter enabled
  GPIO_PinModeSet(BSP_GPIO_PB0_PORT, BSP_GPIO_PB0_PIN,
                  gpioModeInputPullFilter, 1);
  GPIO_PinModeSet(BSP_GPIO_PB1_PORT, BSP_GPIO_PB1_PIN,
                  gpioModeInputPullFilter, 1);

  // Configure LED0 and LED1 as output
  GPIO_PinModeSet(BSP_GPIO_LED0_PORT, BSP_GPIO_LED0_PIN, gpioModePushPull, 0);
  GPIO_PinModeSet(BSP_GPIO_LED1_PORT, BSP_GPIO_LED1_PIN, gpioModePushPull, 0);

  // Enable IRQ for even numbered GPIO pins
  NVIC_EnableIRQ(GPIO_EVEN_IRQn);

  // Enable IRQ for odd numbered GPIO pins
  NVIC_EnableIRQ(GPIO_ODD_IRQn);

  // Enable falling-edge interrupts for PB pins
  GPIO_ExtIntConfig(BSP_GPIO_PB0_PORT,
                    BSP_GPIO_PB0_PIN, BSP_GPIO_PB0_PIN, 0, 1, true);
  GPIO_ExtIntConfig(BSP_GPIO_PB1_PORT,
                    BSP_GPIO_PB1_PIN, BSP_GPIO_PB1_PIN, 0, 1, true);
}

```

**Hàm ngắt của Button 0**

``` c
void GPIO_EVEN_IRQHandler(void)
{
  // Clear all even pin interrupt flags
  GPIO_IntClear(0x5555);
  // Code here
}

```

**Hàm ngắt của Button 1**

``` c
void GPIO_ODD_IRQHandler(void)
{
  // Clear all odd pin interrupt flags
  GPIO_IntClear(0xAAAA);
  // Code here
}

```

Các hàm thay đổi trạng thái LED được cung cấp từ "simplicity\_sdk\_2024.6.2/platform/emlib/inc/em\_gpio.h" của project hiện tại.

``` c
void GPIO_PinOutToggle(GPIO_Port_TypeDef port, unsigned int pin)
void GPIO_PinOutSet(GPIO_Port_TypeDef port, unsigned int pin)

```
