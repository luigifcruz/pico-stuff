# ADC DMA Chain
This is an example of the ADC of the Pico working with chained DMA buffers. This will collect the samples from the ADC using two DMA channels as fast as possible (500ksps). When a DMA is full, the channel will raise an interrupt and start the second channel immediately.

### Dependencies
None.

### Usage
This program will start collecting samples when it receives a char from the virtual serial port. It will also output the following messages:

```txt
Hello from Pi Pico!
Arming DMA.
Start capture.
DMA IRQ 0 [48 47 47]
DMA IRQ 1 [47 47 47]
DMA IRQ 0 [47 47 47]
DMA IRQ 1 [47 47 47]
DMA IRQ 0 [47 47 47]
DMA IRQ 1 [47 47 47]
DMA IRQ 0 [47 47 47]
DMA IRQ 1 [47 47 47]
DMA IRQ 0 [47 47 47]
DMA IRQ 1 [47 47 47]
DMA IRQ 0 [47 47 47]
DMA IRQ 1 [47 47 47]
DMA IRQ 0 [47 47 47]
DMA IRQ 1 [47 47 47]
DMA IRQ 0 [47 47 47]
DMA IRQ 1 [47 47 47]
DMA IRQ 0 [47 47 47]
DMA IRQ 1 [47 47 47]
DMA IRQ 0 [47 47 47]
DMA IRQ 1 [47 47 47]
DMA IRQ 0 [47 47 47]
DMA IRQ 1 [47 47 47]
DMA IRQ 0 [47 47 47]
```