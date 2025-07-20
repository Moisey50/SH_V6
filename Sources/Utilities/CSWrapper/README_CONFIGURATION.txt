References configuration guide:

This lib has reference to another lib (shbaseCLI.dll) from the 'core2013' solution.

1. The name of 'shbaseCLI.dll' file should be SAME in DEBUG (without d-suffix) and RELEASE versions of the Output.

2. Add reference to any version (Debug or Release) of the 'shbaseCLI.dll' file in the project.

3. Open property window of the 'shbaseCLI' reference and change the 'CopyLocal' parameter to the 'False'

4. Open with a text editor the '*.csproj' file of the project.

5. Change reference to the 'shbaseCLI' as follow:
    <Reference Include="shbaseCLI">
      <HintPath>$(OutDir)shbaseCLI.dll</HintPath>
      <Private>False</Private>
    </Reference>

6. Save changes and close the '*.csproj' file of the project. 