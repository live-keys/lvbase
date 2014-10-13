import 'lcvcore' 1.0
import "lcvimgproc" 1.0

// Sample showing how to use 
// Canny Edge Detector algorithm

Row{
    
    property string imagePath : codeDocument.path + '/../_images/caltech_buildings_DSCN0246_small.JPG'
    
    ImRead{
        id : imgSource
        file : parent.imagePath
    }
    
    Canny{
        input: imgSource.output
        threshold1 : 70
        threshold2 : 210
    }
    
    
    
}