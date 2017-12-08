if [ -d doxygen ]; then                                                         
        echo "doxygen directory exists!"                                        
else                                                                            
        curl -O http://ftp.stack.nl/pub/users/dimitri/doxygen-1.8.11.src.tar.gz
        tar -xvf doxygen-1.8.11.src.tar.gz  
        rm doxygen-1.8.11.src.tar.gz                                            
        mv doxygen-1.8.11 doxygen;                                              
fi
