# lsb-bmp
LSB steganography method for BMP files

# Usage:
Hide data in 'hidden' file  into 'container' file and save result in 'output' file.
'output' file default set to 'out.bmp'
```
lsb-bmp -e container hidden [output]
```
Extract data from 'stego' file to 'output' file. 
'output' file default set to 'out_data.bin'
```
lsb-bmp -d stego [output]
```
