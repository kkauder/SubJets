#! /usr/bin/env csh

make bin/DevSubjetWrapper

#parameters
setenv lja antikt
#setenv sja kt
#setenv sja antikt

setenv R 0.4
#setenv S 0.1
#setenv pcmin 2.0
setenv pcmax 30

#setenv pscmin 0.2
#setenv pscmax 30

setenv Nevent 50000

# setenv infile pytest.root
# setenv pjmin 20
# setenv pjmax 30

setenv infile pytest40.root
setenv pjmin 40
setenv pjmax 60

setenv base devPythia${pjmin}${pjmax}

foreach pcmin (0.2 2.0 )
    # foreach S ( 0.05 0.75 0.1 0.125 0.15 0.175 0.2 )
    foreach S ( 0.05 0.1 0.15 0.2 )
	foreach sja ( kt antikt )
	    #construct output name
	    set namebase=${base}_${lja}_R${R}_pcmin${pcmin}_${sja}_SR${S}
	    # echo $namebase
	    # echo SubjetWrapper -N $Nevent -lja $lja -sja $sja -R $R -S $S -pc $pcmin $pcmax -o Results/${namebase}.root 
	    ./bin/DevSubjetWrapper -i $infile -N $Nevent -pj ${pjmin} ${pjmax} -lja $lja -sja $sja -R $R -S $S -pc $pcmin $pcmax -o Results/${namebase}.root >& logs/${namebase}.log &
	end
    end
end







