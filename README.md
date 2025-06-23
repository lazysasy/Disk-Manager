# Disk Manager 📀💡

Disk Manager is an academic prototype that **simulates disk scheduling algorithms, RAID levels, and file-allocation strategies** on embedded hardware (🧠 **Arduino / Raspberry Pi Pico**, ⏱️ **FreeRTOS**, 💾 **SD card**).  
The system visualises operations through a serial interface (or an optional LCD) so students can observe real-time seek movements, block writes, and journalling.




## 🚀 Features

- ✅ **Disk-scheduling algorithms**  
  &nbsp;&nbsp;• FCFS (First-Come, First-Served)  
  &nbsp;&nbsp;• SSTF (Shortest Seek Time First)  
  &nbsp;&nbsp;• SCAN &nbsp;/&nbsp; F-SCAN (Elevator family)

- ✅ **RAID configurations**  
  &nbsp;&nbsp;• RAID 0 – Striping  
  &nbsp;&nbsp;• RAID 1 – Mirroring

- ✅ **File-allocation techniques**  
  &nbsp;&nbsp;• Contiguous allocation  
  &nbsp;&nbsp;• Linked allocation (simulated)

- ✅ **Logging & Journaling** (timestamped `log.txt` on the SD card)  
- ✅ **RTOS task scheduling** for deterministic timing  
- ✅ **SD-card block I/O latency simulation**  
- ✅ **Modular I/O expansion** (LCD, buzzer, serial plotter, …)





## 🧪 Project Components

| Component            | Purpose                               |
|----------------------|---------------------------------------|
| 🖥️ **Raspberry Pi Pico** | Core microcontroller                |
| 💻 **Arduino IDE**        | Development environment            |
| ⏱️ **FreeRTOS**           | Task scheduling / concurrency      |
| 💾 **SD Card Module**     | Disk-block + journal storage       |
| 🔧 **Serial Monitor**     | CLI + live log display             |





## 💻 Installation & Usage

```bash
# 1) Clone the repo
git clone https://github.com/your-username/Disk-Manager.git
cd Disk-Manager

# 2) Open DiskManager.ino in Arduino IDE
#    - Board: Raspberry Pi Pico
#    - Libraries: SD, FreeRTOS (Pico-compatible)

# 3) Upload → open Serial Monitor

# 4) Insert SD card and watch the scheduler in action!

---
## 📚 Algorithms Simulated

- FCFS – Serves requests in the order they arrive  
- SSTF – Chooses the request with the shortest seek distance  
- SCAN / F-SCAN – Elevator-style sweeping across disk tracks  
- RAID 0 / RAID 1 – Striping and mirroring logic simulations  
- File Allocation – Simulates contiguous and linked file block allocation  

---

## 📝 Journaling

Each disk write or update operation is recorded in a `log.txt` file on the SD card, along with:  
- Timestamp  
- Head/cylinder/sector position  
- Operation type (read/write)

This simulates basic journaling and crash recovery mechanisms typically found in real-world file systems.

