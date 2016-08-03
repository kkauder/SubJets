#! /usr/bin/env csh

set Exec = "./bin/Groom"

# make sure executable exists
make $Exec || exit


#parameters
setenv lja antikt
#setenv lja cambri


setenv R 0.4
setenv pcmax 10000

setenv Nevent -1

set noglob 
set input='Data/ppHT/*root'

setenv intype pico
setenv chainname JetTree
setenv etacut 1

setenv embi NONE

setenv pjmin 10
setenv pjmax 2000

####### Initialize condor file
echo ""  > CondorFile
echo "Universe     = vanilla" >> CondorFile
echo "Executable   = ${Exec}" >> CondorFile
echo "getenv = true" >>CondorFile

setenv pcmin 0.2
setenv NameBase Groom
#setenv pcmin 2.0
#setenv NameBase HighConsGroom
setenv trig ppHT

#foreach input ( ${base}* )
#foreach input ( ${base} )
    #construct output name
    # arguments
    # set OutBase=`basename $input | sed 's/.root//g'`
    set OutBase=pp
    set OutName    = Results/${NameBase}_${OutBase}.root

    set LogFile     = logs/${NameBase}_${OutBase}.out
    set ErrFile     = logs/${NameBase}_${OutBase}.err

    set Args = ( -i $input -intype ${intype} -c ${chainname} -trig ${trig} -o ${OutName} -N $Nevent -pj ${pjmin} ${pjmax} -pc ${pcmin} ${pcmax} -lja $lja -ec $etacut -R $R  -embi ${embi} )

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
#end

#condor_submit CondorFile

unset noglob





