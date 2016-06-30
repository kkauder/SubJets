#! /usr/bin/env csh

set Exec = "./bin/Groom"

# make sure executable exists
make $Exec || exit


#parameters
set lja=antikt
#set lja=cambri

set Nevent=-1

# set base=Data/LhcPythia/*root
# set etacut=3
set base=Data/RhicPythia/*root
set etacut=1

# set base=Data/LhcPythiaAndMc_
# set etacut=3
# # set base=Data/PythiaAndMc_0_ptHat
# # set etacut=1

set embi=FAKERHIC

set pjmin=10
set pjmax=2000

####### Initialize condor file
echo ""  > CondorFile
echo "Universe     = vanilla" >> CondorFile
echo "Executable   = ${Exec}" >> CondorFile
echo "getenv = true" >>CondorFile

set R=0.4
#set R=0.2
set ReclusterString=""
set pcmax=10000

#set pcmin=0.2
#set NameBase=Groom
set pcmin=2.0
set NameBase=Hi_Groom

# different declustering algorithm?
set ReclusterString="-rcja antikt"
set NameBase=${NameBase}_RcAntiKt

# set ReclusterString="-rcja kt"
# set NameBase=${NameBase}_RcKt

#foreach input ( ${base}* )
foreach input ( ${base} )
    #construct output name
    # arguments
    set OutBase=`basename $input | sed 's/.root//g'`
    set OutName    = Results/${NameBase}_${OutBase}.root

    set LogFile     = logs/${NameBase}_${OutBase}.out
    set ErrFile     = logs/${NameBase}_${OutBase}.err

    set Args = ( -i $input -o ${OutName} -N $Nevent -pj ${pjmin} ${pjmax} -lja $lja ${ReclusterString} -ec $etacut -R $R -embi $embi -pc $pcmin $pcmax )

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





