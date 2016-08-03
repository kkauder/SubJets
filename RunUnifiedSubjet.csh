#! /usr/bin/env csh

set Exec = "./bin/UnifiedSubjetWrapper"

# make sure executable exists
make $Exec || exit


#parameters
setenv lja antikt
#setenv sja kt
setenv sja antikt

setenv R 0.4
setenv S 0.1
setenv pcmin 0.2
setenv pcmax 1000

#setenv pscmin 2.0
# setenv pscmin 0.2
# setenv pscmax 1000

setenv Nevent -1

setenv base Data/PythiaAndMc_
# setenv pjmin 20
# setenv pjmax 30

setenv pjmin 1
setenv pjmax 100

# setenv infile Data/pytest40.root
# setenv pjmin 40
# setenv pjmax 60

####### Initialize condor file
echo ""  > CondorFile
echo "Universe     = vanilla" >> CondorFile
echo "Executable   = ${Exec}" >> CondorFile
echo "getenv = true" >>CondorFile

foreach input ( ${base}* )
    #construct output name
    # arguments
    set OutBase=`basename $input | sed 's/.root//g'`
    set OutName    = Results/Subjets_${OutBase}.root

    set LogFile     = logs/Subjets_${OutBase}.out
    set ErrFile     = logs/Subjets_${OutBase}.err

    set Args = ( -i $input -o ${OutName} -N $Nevent -pj ${pjmin} ${pjmax} -lja $lja -sja $sja -R $R -S $S -pc $pcmin $pcmax )

    echo "" >> CondorFile
    echo "Output       = ${LogFile}" >> CondorFile
    echo "Error        = ${ErrFile}" >> CondorFile
    echo "Arguments    = ${Args}" >> CondorFile
    echo "Queue" >> CondorFile   

    echo Submitting:
    echo $Exec $Args
    echo "Logging output to " $LogFile
    echo "Logging errors to " $ErrFile
    echo
end

condor_submit CondorFile

# foreach pcmin (0.2 2.0 )
#     # foreach S ( 0.05 0.75 0.1 0.125 0.15 0.175 0.2 )
#     foreach S ( 0.05 0.1 0.15 0.2 )
# 	foreach sja ( kt antikt )
# 	    #construct output name
# 	    set namebase=${base}_${lja}_R${R}_pcmin${pcmin}_${sja}_SR${S}
# 	    # echo $namebase
# 	    # echo SubjetWrapper -N $Nevent -lja $lja -sja $sja -R $R -S $S -pc $pcmin $pcmax -o Results/${namebase}.root 
# 	    ./bin/DevSubjetWrapper -i $infile -N $Nevent -pj ${pjmin} ${pjmax} -lja $lja -sja $sja -R $R -S $S -pc $pcmin $pcmax -o Results/${namebase}.root >& logs/${namebase}.log &
# 	end
#     end
# end






