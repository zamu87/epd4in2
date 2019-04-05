{
   "targets" : [
      {
         "target_name": "epd4in2",
         "sources": ["src/epd4in2.cpp", "src/epdif.cpp"],
		   "libraries": [ "-L/usr/local/lib", "-lwiringPi"],
         "include_dirs" : [
            "<!(node -e \"require('nan')\")"
        ]
      }
   ]
}
