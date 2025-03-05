# RT-enabled ns-3 with NVIDIA Sionna

This is an [ns-3](https://www.nsnam.orghttps:/) version implementing ray tracing for wireless channel simulation using [NVIDIA Sionna RT](https://github.com/NVlabs/sionna).

The integration of Sionna enables **highly accurate propagation modeling** by leveraging GPU-accelerated ray tracing, making it ideal for simulations in complex environments at any frequency, including urban and vehicular scenarios.

For details about this integration running in a possible application example, refer to the following paper:

- [Toward Digital Network Twins: Integrating Sionna RT in ns-3 for 6G Multi-RAT Networks Simulations](https://arxiv.org/abs/2501.00372)

## Key Features

- **Deterministic Wireless Channel Simulations** using the GPU-accelerated RT module in Sionna.
- **Fully Customizable Scenario in Sionna** with the possibility of chosing object meshes, materials (more details about scenes [available here](https://nvlabs.github.io/sionna/api/rt.html)) and antennas.
- **Seamless Mobility Syncronization** between ns-3 Nodes and their correspondent mesh in Sionna.
- **Possibility to run Sionna and ns-3 on two different machines** for example with Sionna in a GPU-powered server farm to reduce computation times

## Installation

ns-3 and Sionna are two separated entities able to communicate with each other via an UDP network socket. For this reason, the installation consists in two steps:

### 1. Installing ns3-rt

A complete ns-3 installation is contained in this repository, as well as the `sionna` module for the integration with Sionna.

Use the following commands to download and build ns3-rt:

```bash
git clone https://github.com/robpegurri/ns3-rt.git
cd ns3-rt
./ns3 configure --disable-python --enable-examples 
./ns3 build
```

### 2. Installing Sionna

To install Sionna, ensure you have Python (versions 3.8 to 3.11) and TensorFlow (versions 2.13 to 2.15) installed. Detailed instructions are available in the official Sionna installation guide. Ubuntu 22.04 is recommended.

**If you are running Sionna on a CPU**, install TensorFlow and LLVM (also required in this case) with:

```bash
sudo apt install llvm
python3 -m pip install tensorflow 
```

and verify the installation with:

```bash
python3 -c "import tensorflow as tf; print(tf.reduce_sum(tf.random.normal([1000, 1000])))"
```

**If you are running Sionna on a GPU**, install the required drivers (refer to your GPU vendor). After the drivers are properly setup, TensorFlow GPU can be installed with:

```bash
python3 -m pip install 'tensorflow[and-cuda]' 
```

and verify the installation with:

```bash
python3 -c "import tensorflow as tf; print(tf.config.list_physical_devices('GPU'))"
```

In case of any issues with TensorFlow or TensorFlow GPU, please refer to the official installation guide [at this page](https://www.tensorflow.org/install).

**At this point, install Sionna** with:

```bash
python3 -m pip install sionna
```

and run the following code in `python3` to check if Sionna was installed properly:

```bash
 >>> import sionna
 >>> print(sionna.__version__)
```

## Running the example

To run the `simple-sionna-example` example, you first need to start Sionna (this example expects Sionna to run locally, see the next section to know how to run Sionna remotely).

Run the Python example script `sionna_server_script.py` from the `/src/sionna` folder with the following command and options:

```bash
python3 'sionna_server_script.py' --local-machine --frequency=2.1e9 --path-to-xml-scenario=scenarios/SionnaExampleScenario/scene.xml
```

After Sionna has started, run the ns-3 simulation in parallel with:

```bash
./ns3 run simple-sionna-example
```

## Simulating with Sionna and ns-3 on separated machines

ns3-sionna was created with the possibility to run Sionna both locally (on the same machine with ns-3) and remotely (in a server with). In your ns-3 script, you can enable this possibility with `SionnaHelper` this way:

```cpp
#include "ns3/sionna-helper.h"
...
SionnaHelper& sionnaHelper = SionnaHelper::GetInstance();
sionnaHelper.SetLocalMachine(false);
sionnaHelper.SetServerIp("YOUR-IP-ADDRESS-HERE");
```

While on Sionna side, just remove the `--local-machine` flag when running the Python script. The default port used by ns3-sionna is **UDP/8103**.

## Notes on using a custom Sionna scene with ns3-rt

ns3-rt links every Node to a specific object in Sionna (associated with a fully customizable mesh). If this mesh is not found in the scene, then Sionna would not know how to calculate any of the requested values by ns3-rt.

In the given example, upon the reception of a *LOC_UPDATE* message from ns3-rt, `sionna_server_script.py` looks for the correspondent object mesh named **car_n**, where **n** is calculated as the ns-3 **Node ID + 1**. The TX and RX antennas are placed on top of the objects (cars, in this case).

To fully understand how to create a custom scene for Sionna, please refer to the [official video tutorial by NVIDIA](https://www.youtube.com/watch?v=7xHLDxUaQ7chttps:/).

## Acknowledgements
If you want to acknowledge our work, please refer to the following pre-print:
```
@misc{pegurri2025digitalnetworktwinsintegrating,
      title={Toward Digital Network Twins: Integrating Sionna RT in ns-3 for 6G Multi-RAT Networks Simulations}, 
      author={Roberto Pegurri and Francesco Linsalata and Eugenio Moro and Jakob Hoydis and Umberto Spagnolini},
      year={2025},
      eprint={2501.00372},
      archivePrefix={arXiv},
      primaryClass={cs.NI},
      url={https://arxiv.org/abs/2501.00372}, 
}
```
