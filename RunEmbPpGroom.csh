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
#unset noglob 
set intype=pico
set chainname=JetTree


# set embbase='Data/NewPicoDst_AuAuCentralMB/newpicoDstcentralMB*.root'
# set embintype=pico
# set embchainname=JetTree
set embi=FAKERHIC
set embintype=tree
set embchainname=tree

set etacut=1

set pjmin=10
set pjmax=2000

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

#foreach embi ( ${embbase} )
    #set noglob 

    #construct output name
    # arguments
    #set OutBase=`basename $embi | sed 's/.root//g'`
    set OutBase=PpEmb
    echo Results/${NameBase}_${OutBase}.root
    set OutName=Results/${NameBase}_${OutBase}.root

    set LogFile     = logs/${NameBase}_${OutBase}.out
    set ErrFile     = logs/${NameBase}_${OutBase}.err

    set Args = ( -i $input -intype ${intype} -c ${chainname} -trig ${trig} -o ${OutName} -N $Nevent -pj ${pjmin} ${pjmax} -pc ${pcmin} ${pcmax} -lja $lja -ec $etacut -R $R  -embi ${embi} -embintype ${embintype} -embc ${embchainname} )

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

condor_submit CondorFile
unset noglob






