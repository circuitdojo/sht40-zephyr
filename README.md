# Sensirion SHT40 for Zephyr

## Use

Incude this project in your `west.yml` to use:

```
manifest:
  projects:
    # SHT40 driver
    - name: sht40
      path: sht40
      revision: main
      url: https://github.com/circuitdojo/sht40-zephyr.git
```

Enable in your `prj.conf`

```
CONFIG_SENSOR=y
CONFIG_SHT40=y
```

Get the binding in your code and go to town:

```
const struct device *dev = DEVICE_DT_GET_ANY(DT_NODELABEL(sht40));
```