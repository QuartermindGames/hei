// Copyright © 2017-2025 Quartermind Games, Mark E. Sowden <hogsy@snortysoft.net>

#pragma once

#include <stddef.h>
#include <stdint.h>

// technically MSVC isn't supported right now by this library,
// at least not until C23 is formally supported, however these
// *may* help it compile...
#if defined( _MSC_VER ) && !defined( __cplusplus )
#	define nullptr   NULL
#	define constexpr const
#endif

/////////////////////////////////////////////////////////////////////////////////////
// System Macros
/////////////////////////////////////////////////////////////////////////////////////

#define QM_OS_SYSTEM_WINDOWS 1
#define QM_OS_SYSTEM_LINUX   2
#define QM_OS_SYSTEM_MACOS   3

#if defined( __linux__ )
#	define QM_OS_SYSTEM_NAME    "Linux"
#	define QM_OS_SYSTEM         QM_OS_SYSTEM_LINUX
#	define QM_OS_SYSTEM_EXE_EXT ""
#	define QM_OS_SYSTEM_LIB_EXT ".so"
#elif defined( __APPLE__ )
#	define QM_OS_SYSTEM_NAME    "macOS"
#	define QM_OS_SYSTEM_OS      QM_OS_SYSTEM_MACOS
#	define QM_OS_SYSTEM_EXE_EXT ""
#	define QM_OS_SYSTEM_LIB_EXT ".so"
#elif defined( _WIN32 )
#	define QM_OS_SYSTEM_NAME    "Windows"
#	define QM_OS_SYSTEM         QM_OS_SYSTEM_WINDOWS
#	define QM_OS_SYSTEM_EXE_EXT ".exe"
#	define QM_OS_SYSTEM_LIB_EXT ".dll"
#else
#	error "Unknown target system!"
#endif

#if !defined( QM_OS_SYSTEM_NAME ) || !defined( QM_OS_SYSTEM ) || !defined( QM_OS_SYSTEM_EXE_EXT ) || !defined( QM_OS_SYSTEM_LIB_EXT )
#	error "Required macros haven't been declared for the target system!"
#endif

/////////////////////////////////////////////////////////////////////////////////////
// Hardware Macros
/////////////////////////////////////////////////////////////////////////////////////

// Endiannes
enum
{
	QM_OS_HARDWARE_LITTLE_ENDIAN,
	QM_OS_HARDWARE_BIG_ENDIAN,
};

// Architecture
enum
{
	QM_OS_HARDWARE_CPU_X86,
	QM_OS_HARDWARE_CPU_X64,
	QM_OS_HARDWARE_CPU_ARM,
	QM_OS_HARDWARE_CPU_ARM64,
};

#if defined( __amd64 ) || defined( __amd64__ ) || defined( _M_X64 ) || defined( _M_AMD64 )
#	define QM_OS_HARDWARE_CPU        QM_OS_HARDWARE_CPU_X64
#	define QM_OS_HARDWARE_CPU_NAME   "AMD64"
#	define QM_OS_HARDWARE_ENDIANNESS QM_OS_HARDWARE_LITTLE_ENDIAN
#elif defined( __arm__ ) || defined( __thumb__ ) || defined( _M_ARM ) || defined( _M_ARMT )
#	define QM_OS_HARDWARE_CPU        QM_OS_HARDWARE_CPU_ARM
#	define QM_OS_HARDWARE_CPU_NAME   "ARM"
#	define QM_OS_HARDWARE_ENDIANNESS QM_OS_HARDWARE_LITTLE_ENDIAN
#elif defined( __aarch64__ )
#	define QM_OS_HARDWARE_CPU        QM_OS_HARDWARE_CPU_ARM64
#	define QM_OS_HARDWARE_CPU_NAME   "ARM64"
#	define QM_OS_HARDWARE_ENDIANNESS QM_OS_HARDWARE_LITTLE_ENDIAN
#elif defined( i386 ) || defined( __i386 ) || defined( __i386__ ) || defined( _M_I86 ) || defined( _M_IX86 ) || defined( _X86_ )
#	define QM_OS_HARDWARE_CPU        QM_OS_HARDWARE_CPU_X86
#	define QM_OS_HARDWARE_CPU_NAME   "INTEL86"
#	define QM_OS_HARDWARE_ENDIANNESS QM_OS_HARDWARE_LITTLE_ENDIAN
#else
#	error "Unsupported CPU type!"
#endif

#if !defined( QM_OS_HARDWARE_CPU ) || !defined( QM_OS_HARDWARE_CPU_NAME ) || !defined( QM_OS_HARDWARE_ENDIANNESS )
#	error "Required macros haven't been declared for the target hardware!"
#endif

/////////////////////////////////////////////////////////////////////////////////////
// Compiler Macros
/////////////////////////////////////////////////////////////////////////////////////

#if defined( __GNUC__ ) || defined( __GNUG__ )
#	define QM_OS_IMPORT
#	define QM_OS_EXPORT __attribute__( ( visibility( "default" ) ) )
#else
#	error "Unsupported compiler!"
#endif

/////////////////////////////////////////////////////////////////////////////////////
// Helper Macros
/////////////////////////////////////////////////////////////////////////////////////

#define QM_OS_ARRAY_ELEMENTS( A )  ( sizeof( A ) / sizeof( *( A ) ) )
#define QM_OS_MAX_ARRAY_INDEX( A ) ( int ) ( QM_OS_ARRAY_ELEMENTS( A ) - 1 )

#define QM_OS_MIN( A, B ) ( ( A ) < ( B ) ? ( A ) : ( B ) )
#define QM_OS_MAX( A, B ) ( ( A ) > ( B ) ? ( A ) : ( B ) )

#define QM_OS_BIT_FLAG( A, B ) A = ( 1U << B )

#define QM_OS_MAGIC_TO_NUM( A, B, C, D ) ( ( ( D ) << 24 ) + ( ( C ) << 16 ) + ( ( B ) << 8 ) + ( A ) )

#define QM_OS_FLOAT_TO_BYTE( A ) ( unsigned char ) ( roundf( ( A ) * 255.0f ) )
#define QM_OS_BYTE_TO_FLOAT( A ) ( ( A ) / ( float ) 255 )

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
