# RT-enabled ns-3 with NVIDIA Sionna

This is an [ns-3](https://www.nsnam.orghttps:/) version implementing ray tracing for wireless channel simulation using [NVIDIA Sionna RT](https://github.com/NVlabs/sionna).

The integration of Sionna enables **highly accurate propagation modeling** by leveraging GPU-accelerated ray tracing, making it ideal for simulations in complex environments at any frequency, including urban and vehicular scenarios.

For details about this integration running in a possible application example, refer to the following paper:

- [Toward Digital Network Twins: Integrating Sionna RT in ns-3 for 6G Multi-RAT Networks Simulations](https://arxiv.org/abs/2501.00372)

## Key Features

- **Deterministic Wireless Channel Simulations** using the GPU-accelerated RT module in Sionna.
- **Fully Customizable Scenario in Sionna** with the possibility of chosing object meshes, materials (more details about scenes [available here](https://nvlabs.github.io/sionna/api/rt.html)) and antennas.
- **Seamless Mobility Syncronization** between ns-3 Nodes and their correspondent mesh in Sionna.

## Installation

ns-3 and Sionna are two separated entities able to communicate with each other via an UDP network socket.

For this reason, the installation consists in two steps:

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
