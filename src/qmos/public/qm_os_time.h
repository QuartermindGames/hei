// Copyright © 2017-2026 Quartermind Games, Mark E. Sowden <markelswo@gmail.com>

#pragma once

/////////////////////////////////////////////////////////////////////////////////////
// Time
/////////////////////////////////////////////////////////////////////////////////////

#if defined( __cplusplus )
extern "C"
{
#endif

	/**
	 * Get time in seconds.
	 *
	 * @return Returns time in seconds, otherwise -1.0 on fail.
	 */
	double qm_os_time_get_seconds();

	/**
	 * Get time in milliseconds.
	 *
	 * @return Returns time in milliseconds, otherwise -1 on fail.
	 */
	long qm_os_time_get_milliseconds();

#if defined( __cplusplus )
};
#endif
