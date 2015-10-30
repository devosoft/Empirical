if [ -d doxygen ]; then                                                         
        echo "doxygen directory exists!"                                        
else                                                                            
        wget http://ftp.stack.nl/pub/users/dimitri/doxygen-1.8.10.src.tar.gz    
        tar -xvf doxygen-1.8.10.src.tar.gz                                      
        rm doxygen-1.8.10.src.tar.gz                                            
        mv doxygen-1.8.10 doxygen;                                              
fi
