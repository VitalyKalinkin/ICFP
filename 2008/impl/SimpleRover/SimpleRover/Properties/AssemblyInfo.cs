// AssemblyInfo.cs created with MonoDevelop
// User: lattyf at 11:45 16.07.2008
//
// To change standard headers go to Edit->Preferences->Coding->Standard Headers
//
using System.Reflection;
using System.Runtime.CompilerServices;

// Information about this assembly is defined by the following attributes. 
// Change them to the values specific to your project.

[assembly: AssemblyTitle("SimpleRover")]
[assembly: AssemblyDescription("")]
[assembly: AssemblyConfiguration("")]
[assembly: AssemblyCompany("")]
[assembly: AssemblyProduct("")]
[assembly: AssemblyCopyright("")]
[assembly: AssemblyTrademark("")]
[assembly: AssemblyCulture("")]

// The assembly version has the format "{Major}.{Minor}.{Build}.{Revision}".
// If the build and revision are set to '*' they will be updated automatically.

[assembly: AssemblyVersion("1.0.*")]

// The following attributes are used to specify the signing key for the assembly, 
// if desired. See the Mono documentation for more information about signing.

[assembly: AssemblyDelaySign(false)]
[assembly: AssemblyKeyFile("")]

// Configure log4net using the .config file
[assembly: log4net.Config.XmlConfigurator(Watch=true)]
