if [ -d doxygen ]; then                                                         
        echo "doxygen directory exists!"                                        
else                                                                            
        curl -O -L https://sourceforge.net/projects/doxygen/files/rel-1.9.7/doxygen-1.9.7.src.tar.gz/download
        tar -xvf download  
        rm download                                            
        mv doxygen-1.9.7 doxygen;                                              
fi
