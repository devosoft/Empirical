if [ -d breathe ]; then                                                         
        echo "breathe directory exists!"                                        
else                                                                            
        wget https://github.com/michaeljones/breathe/archive/v4.1.0.tar.gz
        tar -xvf v4.1.0.tar.gz                                      
        rm v4.1.0.tar.gz
        mv breathe-* breathe                                            
fi
