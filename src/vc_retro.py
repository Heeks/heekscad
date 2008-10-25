# Process the SLN:
f_in  = open('HeeksCAD.sln');
f_vc3 = open('HeeksCAD VC2003.sln', 'w');
f_vc5 = open('HeeksCAD VC2005.sln', 'w');

while (True):
    line = f_in.readline();
    if (len(line) == 0) : break;
    
    if (line == 'Microsoft Visual Studio Solution File, Format Version 10.00\n'):
        f_vc3.write('Microsoft Visual Studio Solution File, Format Version 8.00\n');
        f_vc5.write('Microsoft Visual Studio Solution File, Format Version 9.00\n');
    elif (line == '# Visual C++ Express 2008\n'):
        f_vc5.write('# Visual Studio 2005\n');
    elif (line == 'Project("{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}") = "HeeksCAD", "HeeksCAD.vcproj", "{2702996F-5BCC-436D-A756-D9675FE828A8}"\n'):
        f_vc3.write('Project("{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}") = "HeeksCAD", "HeeksCAD VC2003.vcproj", "{2702996F-5BCC-436D-A756-D9675FE828A8}"\n');
        f_vc5.write('Project("{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}") = "HeeksCAD", "HeeksCAD VC2005.vcproj", "{2702996F-5BCC-436D-A756-D9675FE828A8}"\n');
    else:
        f_vc3.write(line);
        f_vc5.write(line);

f_in.close();
f_vc3.close();
f_vc5.close();

# Process the VCPROJ:
f_in  = open('HeeksCAD.vcproj');
f_vc3 = open('HeeksCAD VC2003.vcproj', 'w');
f_vc5 = open('HeeksCAD VC2005.vcproj', 'w');

while (True):
    line = f_in.readline();
    if (len(line) == 0) : break;

    if (line == '\tVersion="9.00"\n'):
        f_vc3.write('\tVersion="7.10"\n');
        f_vc5.write('\tVersion="8.00"\n');
    else:
        line = line.replace('tinyxml.lib', 'tinyxml2005.lib')
        f_vc3.write(line);
        f_vc5.write(line);

f_in.close();
f_vc3.close();
f_vc5.close();
