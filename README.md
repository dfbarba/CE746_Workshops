# CE 746: Geotechnical Earthquake Engineering  - Workshops

**Repository:** [dfbarba/CE746_Workshops](https://github.com)  
**Course:** CE 746 - Geotechnical Earthquake Engineering

Welcome to the official workshop repository for **CE 746**! This repository contains hands-on tutorials, scripts, and datasets covering essential open-source tools used in seismology and geotechnical earthquake engineering.

---

## Workshop Modules

Below is the overview of the four core workshops included in this repository:

| Folder | Software / Library | 
| :--- | :--- | 
| [`1_GMT`](./1_GMT) | **Generic Mapping Tools (GMT / PyGMT)** | 
| [`2_Obspy`](./2_Obspy) | **ObsPy** | 
| [`3_OpenQuake`](./3_OpenQuake) | **OpenQuake Engine** | 
| [`4_pystrata`](./4_pystrata) | **pyStrata** | 

---

## Repository Structure

```text
CE746_Workshops/
├── 1_GMT/              # Mapping & spatial visualization scripts/notebooks
├── 2_Obspy/            # Strong ground motion records retrieval & signal processing
├── 3_OpenQuake/        # Calculation of distance metrics and application of GMMs
├── 4_pystrata/         # Linear and EQL Site Response Analysis
├── .gitignore          # Files and folders ignored by Git
└── README.md           # Course overview and setup guide
```

---

## Getting Started & Environment Setup

The workshops in this repository rely on **Python** and **Conda** environments. Follow these steps to set up your local machine before class:

### 1. Clone the Repository

### 2. Install Workshop Software
Each workshop has its own dependencies. We recommend isolating your packages using `conda` or `mamba` (via `conda-forge`). Before each session, please follow the specific installation instructions inside the corresponding module folder:

* **Module 1 (GMT / PyGMT):** See instructions inside the [`1_GMT`](./1_GMT) folder.
* **Module 2 (ObsPy):** See instructions inside the [`2_Obspy`](./2_Obspy) folder.
* **Module 3 (OpenQuake):** See instructions inside the [`3_OpenQuake`](./3_OpenQuake) folder.
* **Module 4 (pyStrata):** See instructions inside the [`4_pystrata`](./4_pystrata) folder.

---

## How to Use These Workshops

1. **Pull the latest changes** before each session.
2. Navigate to the corresponding module folder (`1_GMT`, `2_Obspy`, `3_OpenQuake`, or `4_pystrata`).
3. Launch Jupyter Lab or your preferred IDE.
4. Work through the example notebooks and complete any assigned exercises at the bottom of each module.

---

## Questions & Troubleshooting

If you encounter installation issues or bugs in the workshop notebooks:
* Double-check the setup instructions inside the specific module folder.
* Open an [**Issue**](https://github.com/issues) directly in this GitHub repository so classmates can benefit from the solution.
