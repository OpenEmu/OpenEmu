PS3="Select the core to clone: "

select opt in Dolphin PPSSPP UME quit; do

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
    quit)
      break
      ;;
    *) 
      echo "Invalid option $REPLY"
      ;;
  esac
done
