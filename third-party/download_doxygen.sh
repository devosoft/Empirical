if [ -d doxygen ]; then                                                         
        echo "doxygen directory exists!"                                        
else                                                                            
        curl -O -L https://sourceforge.net/projects/doxygen/files/rel-1.9.3/doxygen-1.9.3.src.tar.gz/download
        tar -xvf download  
        rm download                                            
        mv doxygen-1.9.3 doxygen;                                              
fi
