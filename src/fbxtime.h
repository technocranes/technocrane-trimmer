#pragma once

#include <string>
#include "fbxtypes.h"

namespace fbx
{

	////////////////////////////////////////////////////////////////////////////////////
	// FBTime
	////////////////////////////////////////////////////////////////////////////////////

	/** Time modes.
		  * \remarks
		  * EMode \c eNTSCDropFrame is used for broadcasting operations where
		  * clock time must be (almost) in sync with time code. To bring back color
		  * NTSC time code with clock time, this mode drops 2 frames per minute
		  * except for every 10 minutes (00, 10, 20, 30, 40, 50). 108 frames are
		  * dropped per hour. Over 24 hours the error is 2 frames and 1/4 of a
		  * frame. A time-code of 01:00:03:18 equals a clock time of 01:00:00:00
		  *
		  * \par
		  * EMode \c eNTSCFullFrame represents a time address and therefore is NOT
		  * IN SYNC with clock time. A time code of 01:00:00:00 equals a clock time
		  * of 01:00:03:18.
		  *
		  * - \e eDefaultMode
		  * - \e eFrames120			120 frames/s
		  * - \e eFrames100			100 frames/s
		  * - \e eFrames60          60 frames/s
		  * - \e eFrames50          50 frames/s
		  * - \e eFrames48          48 frame/s
		  * - \e eFrames30          30 frames/s (black and white NTSC)
		  * - \e eFrames30Drop		30 frames/s (use when display in frame is selected, equivalent to NTSC drop)
		  * - \e eNTSCDropFrame		~29.97 frames/s drop color NTSC
		  * - \e eNTSCFullFrame		~29.97 frames/s color NTSC
		  * - \e ePAL				25 frames/s	PAL/SECAM
		  * - \e eFrames24			24 frames/s Film/Cinema
		  * - \e eFrames1000		1000 milli/s (use for date time)
		  * - \e eFilmFullFrame		~23.976 frames/s
		  * - \e eCustom            Custom frame rate value
		  * - \e eFrames96			96 frames/s
		  * - \e eFrames72			72 frames/s
		  * - \e eFrames59dot94		~59.94 frames/s
		  * - \e eFrames119dot88	~119.88 frames/s
		  * - \e eModesCount		Number of time modes
		  */
	enum class OFBTimeMode : uint8_t
	{
		eDefaultMode,
		eFrames120,
		eFrames100,
		eFrames60,
		eFrames50,
		eFrames48,
		eFrames30,
		eFrames30Drop,
		eNTSCDropFrame,
		eNTSCFullFrame,
		ePAL,
		eFrames24,
		eFrames1000,
		eFilmFullFrame,
		eCustom,
		eFrames96,
		eFrames72,
		eFrames59dot94,
		eFrames119dot88,
		eModesCount
	};


	//! Time data structure.
	class OFBTime
	{
	private:
		kLongLong	mTime;			//!< Time member.
	public:

		static OFBTimeMode ConvertFrameRateToTimeMode(double frameRate, double precision = 0.000001f);

		//! Different time format available.
		enum ETimeFormats
		{
			eSMPTE,			//!< format as SMPTE
			eFrame,			//!< format as numeric frame
			eDefaultFormat	//!< Default Time format
		};

		/*	Constructor.
		*	\param	pTime	Time to initialize with(default=0).
		*/
		OFBTime(kLongLong pTime = 0);

		/**	Constructor.
		*	\param	pHour		Hour value.
		*	\param	pMinute		Minute value.
		*	\param	pSecond		Second value.
		*	\param	pFrame		Frame value.
		*	\param	pField		Field value.
		*	\param	pTimeMode	Time mode(default=kFBTimeModeDefault).
		*/
		OFBTime(int pHour, int pMinute, int pSecond = 0, int pFrame = 0, int pField = 0, OFBTimeMode pTimeMode = OFBTimeMode::eDefaultMode, double fps=30.0);

		/**	Get time as a string.
		*	\param	pMode		Time mode (default=kFBTimeModeDefault) to use (call FBSystem().GetTransportFps() to the the current UI displayed mode).
		*	\param	pFormat		Format to use for the returned string(default=FBTime::eDefaultFormat).
		*	\return String value of time.
		*/
		std::string GetTimeString(OFBTimeMode pMode = OFBTimeMode::eDefaultMode, ETimeFormats pFormat = eDefaultFormat);

		/**	Set time from string.
		*	\param	pTime	String to set time from.
		*/
		void SetTimeString(const char* pTime);

		/**	Get time (filling separate values)
		*	\retval	pHour			Hour value.
		*	\retval	pMinute			Minute value.
		*	\retval	pSecond			Second value.
		*	\retval	pFrame			Frame value.
		*	\retval	pField			Field value.
		*	\retval	pMilliSecond	MilliSecond value.
		*	\param	pTimeMode		Time mode to get time as.
		*	\return	\b true if an acceptable \e TimeMode value was chosen.
		*/
		bool		GetTime(int& pHour, int& pMinute, int& pSecond, int& pFrame, int& pField, int& pMilliSecond, OFBTimeMode pTimeMode = OFBTimeMode::eDefaultMode);
		/**	Get milliseconds for time.
		*	\return	MilliSeconds value.
		*/
		kLongLong	GetMilliSeconds();

		/**	Set milliseconds time.
		*	\param	pMilliSeconds	MilliSeconds value.
		*/
		void		SetMilliSeconds(kLongLong pMilliSeconds);

		/**	Get time value (long)
		*	\return Time value as long.
		*/
		kLongLong& Get();

		/**	Get time value (long)
		*	\return Time value as long.
		*/
		const kLongLong& Get() const;

		/** Set time value from a long.
		*	\param	pTime	Time value to set.
		*/
		void		Set(kLongLong pTime);

		/**	Get seconds as double.
		*	\return Seconds in double form.
		*/
		double		GetSecondDouble();

		/**	Set seconds from double.
		*	\param	pTime	Time to set seconds from.
		*/
		void		SetSecondDouble(double pTime);

		/**	Set time (from separate values)
		*	\param	pHour			Hour value.
		*	\param	pMinute			Minute value(default=0).
		*	\param	pSecond			Second value(default=0).
		*	\param	pFrame			Frame value(default=0).
		*	\param	pField			Field value(default=0).
		*	\param	pTimeMode		Time mode to get time as(default=kFBTimeModeDefault).
		*/
		void		SetTime(int pHour, int pMinute = 0, int pSecond = 0, int pFrame = 0, int pField = 0, OFBTimeMode pTimeMode = OFBTimeMode::eDefaultMode, double fps=30.0);

		//@{
		/**	Overloaded assignment operators with FBTime objects.
		*	\param	pTime	Time to assign with operator.
		*	\return	Resulting time from operation.
		*/
		OFBTime& operator=	(const OFBTime& pTime);
		OFBTime& operator+=	(const OFBTime& pTime);
		OFBTime& operator-=	(const OFBTime& pTime);
		OFBTime& operator*=	(const OFBTime& pTime);
		OFBTime& operator/=	(const OFBTime& pTime);
		//@}

		//@{
		/**	Overloaded assignment operators with constants.
		*	\param	pConstant	Time to assign with operator.
		*	\return	Resulting time from operation.
		*/
		OFBTime& operator=	(double pConstant);
		OFBTime& operator+=	(double pConstant);
		OFBTime& operator-=	(double pConstant);
		OFBTime& operator*=	(double pConstant);
		OFBTime& operator/=	(double pConstant);
		//@}

		//@{
		/**	Overloaded arithmetic operators with FBTime objects.
		*	\param	pTime	Time to use in operation.
		*	\return	Result in FBTime data.
		*/
		OFBTime	operator- (const OFBTime& pTime);
		OFBTime	operator+ (const OFBTime& pTime);
		OFBTime	operator/ (const OFBTime& pTime);
		OFBTime	operator* (const OFBTime& pTime);
		//@}

		//@{
		/**	Overloaded arithmetic operators with constants.
		*	\param	pConstant	Double constant to add to time.
		*	\return	Result in FBTime data.
		*/
		OFBTime	operator- (double pConstant);
		OFBTime	operator+ (double pConstant);
		OFBTime	operator/ (double pConstant);
		OFBTime	operator* (double pConstant);
		//@}

		//@{
		/** Overloaded comparison operators.
		*	\param	pTime	Time to compare with.
		*	\return	Result of comparison.
		*/
		bool	operator==	(const OFBTime& pTime);
		bool	operator!=	(const OFBTime& pTime);
		bool	operator>=	(const OFBTime& pTime);
		bool	operator<=	(const OFBTime& pTime);
		bool	operator>	(const OFBTime& pTime);
		bool	operator<	(const OFBTime& pTime);
		//@}

		/**	Get the frame count.
		*	With this function, it is possible to obtain the cumulative and local frame counts.
		*	\param	pTimeMode	Time mode to get the constant (default is kFBTimeModeDefault).
		*	\return	Frames per second constant for the specified time mode.
		*/
		kLongLong	GetFrame(OFBTimeMode pTimeMode = OFBTimeMode::eDefaultMode, double fps=30.0);

		/** Set time in frame format.
		* \param pFrames The number of frames.
		* \param pTimeMode The time mode identifier which will dictate the extraction algorithm.
		*/
		void SetFrame(kLongLong pFrames, OFBTimeMode pTimeMode = OFBTimeMode::eDefaultMode, double fps=30.0);

		//! Time constant: Infinity, the largest time value.
		static const OFBTime Infinity;

		//! Time constant: Minus Infinity, the lowest negative time value.
		static const OFBTime MinusInfinity;

		//! Time constant: Zero.
		static const OFBTime Zero;

		//! Time constant: Epsilon, the smallest time increment.
		static const OFBTime Epsilon;

		//! Time constant: One Second.
		static const OFBTime OneSecond;

		//! Time constant: One Minute.
		static const OFBTime OneMinute;

		//! Time constant: One Hour.
		static const OFBTime OneHour;
	};

}
