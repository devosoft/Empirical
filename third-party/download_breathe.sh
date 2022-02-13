if [ -d breathe ]; then                                                         
        echo "breathe directory exists!"                                        
else                                                                            
        wget https://github.com/michaeljones/breathe/archive/refs/tags/v4.32.0.tar.gz
        tar -xvf v4.32.0.tar.gz                                      
        rm v4.32.0.tar.gz
        mv breathe-* breathe                                            
fi
