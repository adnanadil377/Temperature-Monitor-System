# Real-Time Temperature Monitoring & Control System
> **A Dual-Architecture Comparative Study: 8051 (CISC) vs. ARM7 LPC2148 (RISC)**

This repository contains the design, simulation, and firmware for a **Real-Time Temperature Monitoring and Control System**. The system continuously monitors ambient temperature using an **LM35 Analog Temperature Sensor**, displays the readings in real-time on a **16x2 LCD**, and triggers safety actuators (**AC Cooling Relay** and **Buzzer Alarm**) when the temperature exceeds a predefined threshold (50°C).

The core of this project is a hands-on exploration of computer architecture, comparing a legacy CISC-based **8051 microcontroller** implementation against a ported, high-performance RISC-based **ARM7TDMI-S (LPC2148)** implementation.

---

## 🌟 Key Features

- **Real-Time Monitoring**: Accurate temperature sensing with sub-second polling and display updates.
- **Automated Thermal Control**: Actuates a 12V/5V SPDT relay to turn on an "Air Conditioner" (AC) cooling unit once the temperature reaches $\ge 50^\circ\text{C}$.
- **Audible Safety Alarm**: Sounds a piezo buzzer under overheat conditions to alert users.
- **Visual Status Display**: LCD updates dynamically, showing `"Temp: XX C"` on the first line and status (`"Temp Normal"` or `"OVERHEAT !"`) on the second line.
- **Hardware Simulation**: Complete Proteus design schematic (`temp.pdsprj`) supporting interactive firmware simulation.
- **Dual-Architecture Analysis**: Comprehensive mapping and porting notes comparing instruction set efficiency, instruction pipelining, and memory addressing between 8051 and ARM7.

---

## 🛠️ Hardware & Interfacing Specifications (ARM7 LPC2148)

The RISC portion of the project is implemented on the **NXP LPC2148**, a 32-bit ARM7TDMI-S microcontroller operating at up to 60 MHz.

### Pin Mapping Table

| Peripheral | Port Pin | Hardware Role | Connection / Interface |
| :--- | :--- | :--- | :--- |
| **LM35 Sensor** | `P0.28 / AD0.1` | Analog Input | Configured for ADC0 Channel 1 |
| **LCD Register Select (RS)** | `P0.20` | GPIO Output | Command (0) / Data (1) select |
| **LCD Enable (EN)** | `P0.21` | GPIO Output | Latches data on falling edge |
| **LCD Data Bus (D4 - D7)** | `P0.16 - P0.19` | GPIO Output | 4-bit parallel data transmission |
| **AC Relay Actuator** | `P0.22` | GPIO Output | Controls relay driver transistor (NPN) |
| **Buzzer Alarm** | `P1.24` | GPIO Output | Drives active piezoelectric buzzer |

---

## 📐 Mathematical Calibration (ADC to Temperature)

The **LM35** temperature sensor outputs a linear analog voltage of **$10\text{ mV/}^\circ\text{C}$** ($0\text{ V}$ at $0^\circ\text{C}$).

The LPC2148 integrates a **10-bit Successive Approximation ADC**, powered by a reference voltage ($V_{\text{REF}}$) of **$3.3\text{ V}$**. 
The digital value ($ADC_{\text{VAL}}$) from the global data register `AD0GDR` spans from `0` to `1023` ($2^{10} - 1$).

The formula to translate the raw ADC reading into degrees Celsius ($\text{Temp }^\circ\text{C}$) is derived as follows:

$$V_{\text{IN}} = ADC_{\text{VAL}} \times \frac{3.3\text{ V}}{1024}$$

$$\text{Since } 1^\circ\text{C} = 10\text{ mV} = 0.01\text{ V}:$$

$$\text{Temp } (^\circ\text{C}) = \frac{V_{\text{IN}}}{0.01\text{ V}} = ADC_{\text{VAL}} \times \frac{3.3}{1024 \times 0.01}$$

$$\text{Temp } (^\circ\text{C}) = \frac{ADC_{\text{VAL}} \times 330}{1024}$$

The firmware executes this calculation using integer math to prevent the overhead of floating-point units in the embedded pipeline:
```c
temp_int = (adc_val * 330) / 1024;
```

---

## 💻 Firmware Implementation (`main.c`)

The LPC2148 firmware is written in Embedded C and structured as follows:

- **ADC Driver (`read_adc`)**: Configures the control register `AD0CR`, initiates software-triggered conversion, polls the `DONE` flag (bit 31) of `AD0GDR`, and returns the right-shifted 10-bit reading.
- **LCD 4-bit Driver (`lcd_cmd`/`lcd_data`/`lcd_print`)**: Manages Pin states to write instructions and text characters using a 4-bit data bus to reduce GPIO pin count.
- **Control Loop (`main`)**: Continuously samples the temperature, evaluates the safety threshold ($50^\circ\text{C}$), and toggles the relay/buzzer state.

```c
if (temp_int >= 50) {
    IO0SET = AC_RELAY;    // Turn AC ON
    IO1SET = BUZZER;      // Trigger Alarm
    lcd_print("OVERHEAT !");
} else {
    IO0CLR = AC_RELAY;    // Turn AC OFF
    IO1CLR = BUZZER;      // Silence Alarm
    lcd_print("Temp Normal");
}
```

---

## 🔄 Dual-Architecture Comparative Study: CISC vs. RISC

Porting this system from the **8051 (CISC)** architecture to the **ARM7TDMI-S (RISC)** architecture highlights key design paradigms in computer engineering:

| Architectural Metric | Intel 8051 (CISC) | ARM7TDMI-S LPC2148 (RISC) |
| :--- | :--- | :--- |
| **Instruction Set Type** | CISC (Complex Instruction Set Computer) | RISC (Reduced Instruction Set Computer) |
| **Data Bus Width** | 8-bit | 32-bit |
| **Instruction Format** | Variable length (1 to 3 bytes) | Fixed length (32-bit ARM / 16-bit Thumb) |
| **Registers** | Accumulator-based (A, B, DPTR, R0-R7) | Load-Store architecture (16 general registers R0-R15) |
| **Pipelining** | None (Single instruction cycle spans 12 clock cycles) | 3-Stage Pipeline (Fetch, Decode, Execute) |
| **Addressing Modes** | Direct, Indirect, Register, Immediate, External | Load/Store with Offset, Pre/Post-Indexed, Register Offset |
| **ADC Interfacing** | External chip (e.g., ADC0804) over parallel bus | Integrated 10-bit ADC accessible via peripheral bus |

### Key Architectural Insights

1. **Pipeline Stage Analysis**:
   - **8051**: Lacks instruction pipelining. Instructions require 12 or 24 oscillator clock cycles (machine cycles) to complete, bottlenecking instruction throughput.
   - **ARM7**: Utilizes a 3-stage pipeline (Fetch, Decode, Execute). During normal sequential execution, the processor completes one instruction per clock cycle ($CPI \approx 1$), significantly boosting instruction throughput.
2. **Instruction Count & Execution Density**:
   - Performing a 16-bit division (e.g., $(ADC \times 330) / 1024$) in 8051 requires multiple assembly steps, loops, or library calls since it lacks a native 16-bit divide instruction.
   - The 32-bit ARM processor evaluates these mathematical expressions using single-cycle register operations and hardware barrels, reducing the compiler-generated instruction count drastically.
3. **Load-Store Constraints**:
   - **8051** can operate directly on memory locations (e.g., `INC A`, `MOV direct, A`).
   - **ARM7** requires all data to be loaded into registers before execution (`LDR`), processed inside registers, and then written back to memory (`STR`). While this requires more instruction entries for memory access, the uniform cycle duration and high clock speeds offset the load-store instruction overhead.

---

## 📂 Project Structure

```
├── main.c              # Core ARM7 LPC2148 firmware source code
├── Startup.s           # ARM assembly startup file initializing registers, stacks, and vectors
├── temp.uvproj         # Keil uVision Project File
├── temp.uvopt          # Keil Project configuration settings
├── temp.pdsprj         # Proteus Design Suite schematic and simulation file
├── temp.hex            # Compiled Intel HEX binary ready for flashing or simulation
└── README.md           # Project documentation and architectural overview
```

---

## 🚀 How to Run and Simulate

### 1. Compile with Keil uVision
1. Open [temp.uvproj](file:///C:/Users/SAHEER/OneDrive/Desktop/sem%206/embedded/temp/temp.uvproj) in **Keil uVision**.
2. Select target `Target 1` and click **Build Target** (`F7`) to compile the project.
3. The build process creates `temp.hex` and `temp.axf` inside the directory.

### 2. Simulate in Proteus
1. Launch **Proteus Design Suite** and open [temp.pdsprj](file:///C:/Users/SAHEER/OneDrive/Desktop/sem%206/embedded/temp/temp.pdsprj).
2. Double-click the LPC2148 microcontroller schematic symbol.
3. Under the **Program File** field, browse and select the compiled [temp.hex](file:///C:/Users/SAHEER/OneDrive/Desktop/sem%206/embedded/temp/temp.hex) file.
4. Press the **Play/Simulation** button at the bottom-left of the window.
5. Interact with the LM35 up/down voltage arrows to alter temperature and observe LCD changes and actuator activation.
