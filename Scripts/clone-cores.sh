PS3="Select the core to clone: "

select opt in Dolphin PPSSPP UME PCSX2 quit; do

  case $opt in
    Dolphin)
      git clone git@github.com:OpenEmu/dolphin.git --recursive
      break
      ;;
    PPSSPP)
      git clone git@github.com:OpenEmu/PPSSPP-Core.git --recursive
      break
      ;;
    UME)
      git clone git@github.com:OpenEmu/UME-Core.git --recursive
      break
      ;;
    PCSX2)
      git clone git@github.com:OpenEmu/PCSX2-Core.git --recursive
      break
      ;;
    quit)
      break
      ;;
    *) 
      echo "Invalid option $REPLY"
      ;;
  esac
done
