cmd_/home/kdz7/TP2/messag/Module.symvers :=  sed 's/ko$$/o/'  /home/kdz7/TP2/messag/modules.order | scripts/mod/modpost -m      -o /home/kdz7/TP2/messag/Module.symvers -e -i Module.symvers -T - 
