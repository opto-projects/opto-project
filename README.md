# opto project

This application is for Camera based Optics Imaging Platform 

the app is a standalone desktop application for calculation of optics matrix. It can process a single monochrome image or a data stream from some types of industrial cameras.

# 项目结构
- src 文件夹，这个文件夹用来放置应用相关的源代码
- doc 文件夹，这个文件夹用来放置所有的文档
- lib 文件夹，这个文件夹用来放置所有的第三方库以及自定义的核心库
    - optocore  用来实现插件的实现
- doc 文件夹，这个文件夹用来放置所有的文档
- plugins 文件夹，这个文件夹是扩展应用的插件代码
- cmake 这个文件夹放置了一些封装好的 cmake 文件

opto-project/  
├── CMakeLists.txt  
├── README.md  
├── cmake/  
├── bin/  
├── doc/  
├── lib/  
│   ├── optocore/  
│   │   ├── CMakeLists.txt  
│   ├── qcustomplot/  
├── plugins/  
│   ├── CMakeLists.txt  
│   ├── virtual_acquisition_system/  
│   │   ├── CMakeLists.txt  
│   ├── darkimage_processing/  
│   ├── Psf_processing/  
├── src/  
│   ├── aboutdata/  
│   ├── app/  
│   ├── default/  
│   ├── icon/  




