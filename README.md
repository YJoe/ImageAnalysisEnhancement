# ImageAnalysisEnhancement
A university image analysis project to correct the following panda image using OpenCV to perform spatial and frequency domain corrections.

![alt text](https://github.com/YJoe/ImageAnalysisEnhancement/blob/master/ImageAnalysisEnhancement/data/PandaNoise.bmp)

### Frequency Enhancement
The DFT of the panda is taken using OpenCV functionality.

![alt text](https://github.com/YJoe/ImageAnalysisEnhancement/blob/master/ImageAnalysisEnhancement/readme/DFT.PNG)

The DFT of the panda is shifted to visualise low frequency data in the center and high frequency data in the corners.

![alt text](https://github.com/YJoe/ImageAnalysisEnhancement/blob/master/ImageAnalysisEnhancement/readme/ShiftedDFT.PNG)

By applying various masks to the DFT, shifting back and taking the inverse DFT, the image quality has (arguably!) been improved.

##### Low Pass Filter
![alt text](https://github.com/YJoe/ImageAnalysisEnhancement/blob/master/ImageAnalysisEnhancement/readme/LowPass.PNG)

##### High Pass Filter
![alt text](https://github.com/YJoe/ImageAnalysisEnhancement/blob/master/ImageAnalysisEnhancement/readme/HighPass.PNG)

##### Single Band Stop Filter
![alt text](https://github.com/YJoe/ImageAnalysisEnhancement/blob/master/ImageAnalysisEnhancement/readme/SingleBandStop.PNG)

##### Double Band Stop Filter
![alt text](https://github.com/YJoe/ImageAnalysisEnhancement/blob/master/ImageAnalysisEnhancement/readme/DoubleBandStop.PNG)

##### Tripple Band Stop Filter
![alt text](https://github.com/YJoe/ImageAnalysisEnhancement/blob/master/ImageAnalysisEnhancement/readme/TrippleBandStop.PNG)

##### A Very Specific Filter
![alt text](https://github.com/YJoe/ImageAnalysisEnhancement/blob/master/ImageAnalysisEnhancement/readme/HighlightStop.PNG)

Though tedious to make, the final specific filter saw great results and despite a large amount of static still visible on the panda, the image has been improved greatly by removing the common frequencies across the image
