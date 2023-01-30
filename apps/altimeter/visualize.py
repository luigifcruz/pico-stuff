import numpy as np 
import matplotlib.pyplot as plt
from matplotlib.ticker import FormatStrFormatter

# Convert HEX file to BIN.
# cat D4.hex | xxd -r -p > D4.bin

data = np.fromfile('./D5.bin', dtype=np.float32)
temp_x = data[0::2]
pres_x = data[1::2]
alt_x = ((((1013.25 / pres_x) ** (1/5.257)) - 1) * (temp_x + 273.15)) / 0.0065
y = np.linspace(0, len(temp_x) / 4, len(temp_x)) 
max_alt_del = np.max(alt_x) - np.min(alt_x)

fig = plt.figure()

ax = fig.add_subplot(111, label="1")
ax2 = fig.add_subplot(111, label="2", frame_on=False)

ax.set_title(f"Kite Altimeter - 1st Flight | Max Altitude Delta = {max_alt_del:.2f} m")
ax.plot(y, temp_x, color="fuchsia", linewidth=1)
ax.set_ylabel('Temperature (ºC)', color="fuchsia")
ax.yaxis.set_label_position("right")
ax.yaxis.tick_right()
ax.tick_params(axis='y', colors="fuchsia")
ax.yaxis.set_major_formatter(FormatStrFormatter('%.0f'))

ax2.plot(y, alt_x, color="#50fa7b", linewidth=1)
ax2.set_xlabel("Time (s)")
ax2.set_ylabel("Altitude (m)", color="#50fa7b")
ax2.tick_params(axis='y', colors="#50fa7b")
ax2.yaxis.set_major_formatter(FormatStrFormatter('%.0f'))

plt.savefig('altitude.png', dpi=300)

plt.show()