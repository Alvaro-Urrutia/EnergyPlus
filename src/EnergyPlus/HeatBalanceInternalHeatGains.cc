// ObjexxFCL Headers
#include <ObjexxFCL/FArray.functions.hh>
#include <ObjexxFCL/FArray1D.hh>

// EnergyPlus Headers
#include <HeatBalanceInternalHeatGains.hh>
#include <DataHeatBalance.hh>
#include <DataPrecisionGlobals.hh>
#include <InputProcessor.hh>
#include <UtilityRoutines.hh>

namespace EnergyPlus {

// outside of Module

void
SetupZoneInternalGain(
	int const ZoneNum,
	Fstring const & cComponentObject, // object class name for device contributing internal gain
	Fstring const & cComponentName, // user unique name for device
	int const IntGainComp_TypeOfNum,
	Optional< Real64 > ConvectionGainRate, // pointer target for remote convection gain value to be accessed
	Optional< Real64 > ReturnAirConvectionGainRate,
	Optional< Real64 > ThermalRadiationGainRate, // pointer target for remote IR radiation gain value to be accessed
	Optional< Real64 > LatentGainRate,
	Optional< Real64 > ReturnAirLatentGainRate,
	Optional< Real64 > CarbonDioxideGainRate,
	Optional< Real64 > GenericContamGainRate
)
{

	// SUBROUTINE INFORMATION:
	//       AUTHOR         B. Griffith
	//       DATE WRITTEN   November 2011
	//       MODIFIED       na
	//       RE-ENGINEERED  na

	// PURPOSE OF THIS SUBROUTINE:
	// provide a general interface for setting up devices with internal gains

	// METHODOLOGY EMPLOYED:
	// use pointers to access gain rates in device models
	// devices are internal gains like people, lights, electric equipment
	// and HVAC components with skin loss models like thermal tanks, and power conditioning.

	// REFERENCES:
	// na

	// Using/Aliasing
	using namespace DataPrecisionGlobals;
	using namespace DataHeatBalance;
	using InputProcessor::MakeUPPERCase;
	using InputProcessor::SameString;

	// Locals
	// SUBROUTINE ARGUMENT DEFINITIONS:

	// SUBROUTINE PARAMETER DEFINITIONS:
	int const DeviceAllocInc( 100 );

	// INTERFACE BLOCK SPECIFICATIONS:
	// na

	// DERIVED TYPE DEFINITIONS:
	// na

	// SUBROUTINE LOCAL VARIABLE DECLARATIONS:
	int IntGainsNum;
	bool FoundIntGainsType;
	bool FoundDuplicate;
	Fstring UpperCaseObjectType( MaxNameLength );
	Fstring UpperCaseObjectName( MaxNameLength );

	// Object Data
	FArray1D< GenericComponentZoneIntGainStruct > TempGenDeviceIntGainsArr;

	FoundIntGainsType = false;
	FoundDuplicate = false;
	UpperCaseObjectType = MakeUPPERCase( cComponentObject );
	UpperCaseObjectName = MakeUPPERCase( cComponentName );

	// Check if IntGainComp_TypeOfNum and cComponentObject are consistent
	if ( ! SameString( UpperCaseObjectType, ZoneIntGainDeviceTypes( IntGainComp_TypeOfNum ) ) ) {
		ShowSevereError( "SetupZoneInternalGain: developer error, trapped inconsistent internal gains object types" " sent to SetupZoneInternalGain" );
		ShowContinueError( "Object type character = " + trim( cComponentObject ) );
		ShowContinueError( "Type of Num object name = " + trim( ZoneIntGainDeviceTypes( IntGainComp_TypeOfNum ) ) );
		return;
	}

	for ( IntGainsNum = 1; IntGainsNum <= ZoneIntGain( ZoneNum ).NumberOfDevices; ++IntGainsNum ) {
		if ( ( ZoneIntGain( ZoneNum ).Device( IntGainsNum ).CompObjectType == UpperCaseObjectType ) && ( ZoneIntGain( ZoneNum ).Device( IntGainsNum ).CompTypeOfNum == IntGainComp_TypeOfNum ) ) {
			FoundIntGainsType = true;
			if ( ZoneIntGain( ZoneNum ).Device( IntGainsNum ).CompObjectName == UpperCaseObjectName ) {
				FoundDuplicate = true;
				break;
			}
		}
	}

	if ( FoundDuplicate ) {
		ShowSevereError( "SetupZoneInternalGain: developer error, trapped duplicate internal gains sent to SetupZoneInternalGain" );
		ShowContinueError( "The duplicate object user name =" + trim( cComponentName ) );
		ShowContinueError( "The duplicate object type = " + trim( cComponentObject ) );
		ShowContinueError( "This internal gain will not be modeled, and the simulation continues" );
		return;
	}

	if ( ZoneIntGain( ZoneNum ).NumberOfDevices == 0 ) {
		ZoneIntGain( ZoneNum ).Device.allocate( DeviceAllocInc );
		ZoneIntGain( ZoneNum ).NumberOfDevices = 1;
		ZoneIntGain( ZoneNum ).MaxNumberOfDevices = DeviceAllocInc;
	} else {
		if ( ZoneIntGain( ZoneNum ).NumberOfDevices + 1 > ZoneIntGain( ZoneNum ).MaxNumberOfDevices ) {
			TempGenDeviceIntGainsArr.allocate( ZoneIntGain( ZoneNum ).MaxNumberOfDevices + DeviceAllocInc );
			TempGenDeviceIntGainsArr( {1,ZoneIntGain( ZoneNum ).NumberOfDevices} ) = ZoneIntGain( ZoneNum ).Device( {1,ZoneIntGain( ZoneNum ).NumberOfDevices} );
			ZoneIntGain( ZoneNum ).Device.deallocate();
			ZoneIntGain( ZoneNum ).Device.allocate( ZoneIntGain( ZoneNum ).MaxNumberOfDevices + DeviceAllocInc );
			ZoneIntGain( ZoneNum ).MaxNumberOfDevices += DeviceAllocInc;
			ZoneIntGain( ZoneNum ).Device( {1,ZoneIntGain( ZoneNum ).NumberOfDevices} ) = TempGenDeviceIntGainsArr( {1,ZoneIntGain( ZoneNum ).NumberOfDevices} );
			TempGenDeviceIntGainsArr.deallocate();
		}
		++ZoneIntGain( ZoneNum ).NumberOfDevices;
	}

	ZoneIntGain( ZoneNum ).Device( ZoneIntGain( ZoneNum ).NumberOfDevices ).CompObjectType = UpperCaseObjectType;
	ZoneIntGain( ZoneNum ).Device( ZoneIntGain( ZoneNum ).NumberOfDevices ).CompObjectName = UpperCaseObjectName;
	ZoneIntGain( ZoneNum ).Device( ZoneIntGain( ZoneNum ).NumberOfDevices ).CompTypeOfNum = IntGainComp_TypeOfNum;

	// note pointer assignments in code below!
	if ( present( ConvectionGainRate ) ) {
		ZoneIntGain( ZoneNum ).Device( ZoneIntGain( ZoneNum ).NumberOfDevices ).PtrConvectGainRate >>= ConvectionGainRate;
	} else {
		ZoneIntGain( ZoneNum ).Device( ZoneIntGain( ZoneNum ).NumberOfDevices ).PtrConvectGainRate >>= ZeroPointerVal;
	}

	if ( present( ReturnAirConvectionGainRate ) ) {
		ZoneIntGain( ZoneNum ).Device( ZoneIntGain( ZoneNum ).NumberOfDevices ).PtrReturnAirConvGainRate >>= ReturnAirConvectionGainRate;
	} else {
		ZoneIntGain( ZoneNum ).Device( ZoneIntGain( ZoneNum ).NumberOfDevices ).PtrReturnAirConvGainRate >>= ZeroPointerVal;
	}

	if ( present( ThermalRadiationGainRate ) ) {
		ZoneIntGain( ZoneNum ).Device( ZoneIntGain( ZoneNum ).NumberOfDevices ).PtrRadiantGainRate >>= ThermalRadiationGainRate;
	} else {
		ZoneIntGain( ZoneNum ).Device( ZoneIntGain( ZoneNum ).NumberOfDevices ).PtrRadiantGainRate >>= ZeroPointerVal;
	}

	if ( present( LatentGainRate ) ) {
		ZoneIntGain( ZoneNum ).Device( ZoneIntGain( ZoneNum ).NumberOfDevices ).PtrLatentGainRate >>= LatentGainRate;
	} else {
		ZoneIntGain( ZoneNum ).Device( ZoneIntGain( ZoneNum ).NumberOfDevices ).PtrLatentGainRate >>= ZeroPointerVal;
	}

	if ( present( ReturnAirLatentGainRate ) ) {
		ZoneIntGain( ZoneNum ).Device( ZoneIntGain( ZoneNum ).NumberOfDevices ).PtrReturnAirLatentGainRate >>= ReturnAirLatentGainRate;
	} else {
		ZoneIntGain( ZoneNum ).Device( ZoneIntGain( ZoneNum ).NumberOfDevices ).PtrReturnAirLatentGainRate >>= ZeroPointerVal;
	}

	if ( present( CarbonDioxideGainRate ) ) {
		ZoneIntGain( ZoneNum ).Device( ZoneIntGain( ZoneNum ).NumberOfDevices ).PtrCarbonDioxideGainRate >>= CarbonDioxideGainRate;
	} else {
		ZoneIntGain( ZoneNum ).Device( ZoneIntGain( ZoneNum ).NumberOfDevices ).PtrCarbonDioxideGainRate >>= ZeroPointerVal;
	}

	if ( present( GenericContamGainRate ) ) {
		ZoneIntGain( ZoneNum ).Device( ZoneIntGain( ZoneNum ).NumberOfDevices ).PtrGenericContamGainRate >>= GenericContamGainRate;
	} else {
		ZoneIntGain( ZoneNum ).Device( ZoneIntGain( ZoneNum ).NumberOfDevices ).PtrGenericContamGainRate >>= ZeroPointerVal;
	}

}

} // EnergyPlus