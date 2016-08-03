#!/usr/bin/env csh

### Combine with
# foreach n ( `seq 1 3` )
# hadd -f AjResults/rndm$n/R0.2_HC30_Presel.root AjResults/rndm$n/R0.2_HC30_Small_Clean8*root
# end


set Exec = "./bin/PicoAj"

# make sure executable exists
make $Exec || exit

# choose R or Pt options
set RMod = ""
# set RMod = R0.2_
# set RMod = Pt1_

# Input files
# set base = SmallAuAu/Small_Clean8
set base = Data/SmallNPE18/Small_AuAu11*
#set base = Data/SmallNPE18/Small_AuAu11PicoNPE18_Cent8_[0-9].root

#foreach randomoff ( `seq 0 9 ` )
#end # foreach randomoff

# choose random seed offset
set randomoff = 0
if { ( echo $1 | egrep -q '^[0-9]$' ) } then
    set randomoff = $1
else if ( $#argv > 0 ) then
    echo illegal arguments $argv
    exit -1    
endif

if ( $randomoff < 0  || $randomoff > 9 ) then
    echo illegal randomoff $randomoff
    exit -1
endif

set rndname = ""
if ( $randomoff > 0 ) then
    set rndname = rndm${randomoff}
    mkdir -pv AjResults/${rndname}
    mkdir -pv logs/${rndname}
endif

####### Initialize condor file
echo ""  > CondorFile
echo "Universe     = vanilla" >> CondorFile
echo "Executable   = ${Exec}" >> CondorFile
echo "getenv = true" >>CondorFile
# Notification doesn't seem to work, and is spammy anyway
# echo "Notification = Complete" >> CondorFile
# echo "Notify_user  = kkauder@gmail.com"  >> CondorFile

foreach input ( ${base}* )
    # arguments
    set OutBase=`basename $input | sed 's/.root//g'`
    #set OutName    = AjResults/${rndname}/${RMod}Presel_AuAuAj_${OutBase}.root
    #set OutName    = AjResults/${rndname}/${RMod}NewCode_HC100_${OutBase}.root
    #set OutName    = AjResults/${rndname}/${RMod}FixedTowers_${OutBase}.root
    set OutName    = AjResults/${rndname}/${RMod}NPE18_HC100_${OutBase}.root
	
    set TriggerName = HT
    set Files      = ${input}
    
    # Logfiles.
    # set LogFile     = logs/${rndname}/${RMod}Presel_AuAuAj_${OutBase}.out
    # set ErrFile     = logs/${rndname}/${RMod}Presel_AuAuAj_${OutBase}.err
    # set LogFile     = logs/${rndname}/${RMod}NewCode_HC100_${OutBase}.out
    # set ErrFile     = logs/${rndname}/${RMod}NewCode_HC100_${OutBase}.err
    # set LogFile     = logs/${rndname}/${RMod}FixedTowers_${OutBase}.out
    # set ErrFile     = logs/${rndname}/${RMod}FixedTowers_${OutBase}.err
    set LogFile     = logs/${rndname}/${RMod}NPE18_HC100_${OutBase}.out
    set ErrFile     = logs/${rndname}/${RMod}NPE18_HC100_${OutBase}.err

    ### hand to condor
    set Args = ( $OutName $TriggerName $Files 0 0 )
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
    
end # foreach input

condor_submit CondorFile


