mkdir External

# Get common dependencies
wget -O ./External/Common.zip 192.168.1.66:8090/0.1/Downloads/Common.zip

# Download x86_64-Windows-MSVC dependencies
wget -O ./External/x86_64-Windows-MSVC.zip 192.168.1.66:8090/0.1/Downloads/x86_64-Windows-MSVC.zip

# Download x86_64-Windows-MSVC dependencies
wget -O ./External/x86_64-Windows-MinGW.zip 192.168.1.66:8090/0.1/Downloads/x86_64-Windows-MinGW.zip

# Download x86_64-Linux-GCC dependencies
wget -O ./External/x86_64-Linux-GCC.zip 192.168.1.66:8090/0.1/Downloads/x86_64-Linux-GCC.zip

# Unzip the dependencies
unzip ./External/Common.zip -d ./External
unzip ./External/x86_64-Windows-MSVC.zip -d ./External
unzip ./External/x86_64-Windows-MinGW.zip -d ./External
unzip ./External/x86_64-Linux-GCC.zip -d ./External

# Delete the downloaded zips
rm -f ./External/Common.zip
rm -f ./External/x86_64-Windows-MSVC.zip
rm -f ./External/x86_64-Windows-MinGW.zip
rm -f ./External/x86_64-Linux-GCC.zip
