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
#set input='Data/ppHT/*root'

setenv intype tree
setenv chainname TriggeredTree
setenv etacut 1

#setenv embi NONE
setenv embi 'Data/NewPicoDst_AuAuCentralMB/*root'
setenv embintype pico
setenv embc JetTree
setenv embtrig MB

setenv pjmin 10
setenv pjmax 2000

####### Initialize condor file
echo ""  > CondorFile
echo "Universe     = vanilla" >> CondorFile
echo "Executable   = ${Exec}" >> CondorFile
echo "getenv = true" >>CondorFile

# setenv pcmin 0.2
# setenv NameBase Groom
setenv pcmin 2.0
setenv NameBase Hi_Groom

#setenv trig ppHT
setenv nmix 20

#foreach inputbase ( `find Data/ppHT/ -name *root | cut -c 1-23 | uniq` )
foreach inputbase ( `find AjResults -name Tow0_Eff0_\*_of_12\*root | sed 's/.root//g'` )
    set input=$inputbase*root

    #construct output name
    # arguments
    # set OutBase=`basename $input | sed 's/.root//g'`
    # set OutBase=pp
    #set OutName    = Results/${NameBase}_${OutBase}.root
    set OutName    = Results/${NameBase}_pp_`basename ${inputbase}`.root
    #echo $input '->' $OutName

    set LogFile     = logs/${NameBase}_pp_`basename ${inputbase}`.out
    set ErrFile     = logs/${NameBase}_pp_`basename ${inputbase}`.err

    set Args = ( -i $input -intype ${intype} -c ${chainname} -o ${OutName} -N $Nevent -pj ${pjmin} ${pjmax} -pc ${pcmin} ${pcmax} -lja $lja -ec $etacut -R $R  -embi ${embi} -nmix ${nmix} -embintype ${embintype} -embc ${embc} -embtrig ${embtrig} )

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

unset noglob





