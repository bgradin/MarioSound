#define HALF_NOTE 1.059463094359 // HALF_NOTE ^ 12 = 2
#define PI 3.14159265358979
#define NOTE_DURATION 0.014
#define SAMPLES_PER_SECOND 44000
#define AMPLITUDE 1

#include <Windows.h>
#include <math.h>
#include <vector>
using namespace std;

class SoundEffect
{
public:
	SoundEffect() {}
	SoundEffect(const int noteInfo[], const int arraySize)
	{
		// Initialize the sound format we will request from sound card
		m_waveFormat.wFormatTag = WAVE_FORMAT_PCM;        // Uncompressed sound format
		m_waveFormat.nChannels = 1;                       // 1 = Mono, 2 = Stereo
		m_waveFormat.wBitsPerSample = 8;                  // Bits per sample per channel
		m_waveFormat.nSamplesPerSec = SAMPLES_PER_SECOND; // Samples Per Second
		m_waveFormat.nBlockAlign = 1;
		m_waveFormat.nAvgBytesPerSec = SAMPLES_PER_SECOND;
		m_waveFormat.cbSize = 0;

		int pushedNotes = 0;

		vector<note> noteVector;

		noteVector.push_back(note(noteInfo[pushedNotes * 2], noteInfo[(pushedNotes * 2) + 1]));
		pushedNotes++;

		// Fill the sound buffer array with the sound values
		while (noteVector.size() != 0)
		{
			double level = 0;

			for (unsigned int j = 0; j < noteVector.size(); j++)
			{
				level += noteVector[j].currentLevel();

				if (noteVector[j].totallyDone())
					noteVector.erase(noteVector.begin() + j--);
			}

			if ((noteVector.size() == 0 || noteVector.back().partiallyDone()) && pushedNotes < arraySize / 2)
			{
				noteVector.push_back(note(noteInfo[pushedNotes * 2], noteInfo[(pushedNotes * 2) + 1]));
				pushedNotes++;
			}

			m_data.push_back((char) (127 * level + 128));
		}
	}
	SoundEffect(SoundEffect& otherInstance)
	{
		m_data = otherInstance.m_data;
		m_waveFormat = otherInstance.m_waveFormat;
	}

	SoundEffect& operator=(SoundEffect& otherInstance)
	{
		m_data = otherInstance.m_data;
		m_waveFormat = otherInstance.m_waveFormat;

		return *this;
	}

	void Play()
	{
		// Create our "Sound is Done" event
		m_done = CreateEvent (0, FALSE, FALSE, 0);

		if (m_done == 0)
		{
			cout << "Couldn't create event." << endl;
			return;
		}

		// Open the audio device
		if (waveOutOpen(&m_waveOut, 0, &m_waveFormat, (DWORD) m_done, 0, CALLBACK_EVENT) != MMSYSERR_NOERROR) 
		{
			cout << "Sound card cannot be opened." << endl;
			return;
		}

		// Create the wave header for our sound buffer
		m_waveHeader.lpData = &m_data[0];
		m_waveHeader.dwBufferLength = m_data.size();
		m_waveHeader.dwFlags = 0;
		m_waveHeader.dwLoops = 0;

		// Prepare the header for playback on sound card
		if (waveOutPrepareHeader(m_waveOut, &m_waveHeader, sizeof(m_waveHeader)) != MMSYSERR_NOERROR)
		{
			cout << "Error preparing Header!" << endl;
			return;
		}

		// Play the sound!
		ResetEvent(m_done); // Reset our Event so it is non-signaled, it will be signaled again with buffer finished

		if (waveOutWrite(m_waveOut, &m_waveHeader, sizeof(m_waveHeader)) != MMSYSERR_NOERROR)
		{
			cout << "Error writing to sound card!" << endl;
			return;
		}

		// Wait until sound finishes playing
		if (WaitForSingleObject(m_done, INFINITE) != WAIT_OBJECT_0)
		{
			cout << "Error waiting for sound to finish" << endl;
			return;
		}

		// Unprepare our wav header
		if (waveOutUnprepareHeader(m_waveOut, &m_waveHeader,sizeof(m_waveHeader)) != MMSYSERR_NOERROR)
		{
			cout << "Error unpreparing header!" << endl;
			return;
		}

		// Close the wav device
		if (waveOutClose(m_waveOut) != MMSYSERR_NOERROR)
		{
			cout << "Sound card cannot be closed!" << endl;
			return;
		}

		// Release our event handle
		CloseHandle(m_done);
	}

private:
	class note
	{
	public:
		note(int distanceFromA1, int duration)
		{
			m_duration = (duration < 2) ? duration + 1 : duration;
			m_period = (1.0 / (110.0 * pow(HALF_NOTE, distanceFromA1))) * SAMPLES_PER_SECOND;

			m_totalSamples = (int) floor(SAMPLES_PER_SECOND * NOTE_DURATION);
			if (m_duration > 1)
				m_totalSamples *= m_duration;

			m_samplesPlayed = 0;
		}
		note(note& rhs)
		{
			m_totalSamples = rhs.m_totalSamples;
			m_samplesPlayed = rhs.m_samplesPlayed;
			m_period = rhs.m_period;
			m_duration = rhs.m_duration;
		}
		double currentLevel()
		{
			m_samplesPlayed++;
			return quarterDone()
				? 0
				: AMPLITUDE
					* pow(-1.0, floor(m_samplesPlayed / m_period))
					* ((m_period
						- (((double) m_samplesPlayed) - (m_period * floor(m_samplesPlayed / m_period))))
						/ m_period);
		}
		bool totallyDone()
		{
			return m_samplesPlayed >= m_totalSamples;
		}
		bool partiallyDone()
		{
			return totallyDone() || (m_duration == 0 && quarterDone());
		}

	private:
		bool quarterDone()
		{
			return (m_duration > 4) ? m_samplesPlayed >= m_totalSamples / 4 : m_samplesPlayed >= (NOTE_DURATION * SAMPLES_PER_SECOND);
		}

		int m_totalSamples, m_samplesPlayed, m_duration;
		double m_period;
	};

	HWAVEOUT m_waveOut; // Handle to sound card output
	WAVEFORMATEX m_waveFormat; // The sound format
	WAVEHDR m_waveHeader; // WAVE header for our sound data
	HANDLE m_done; // Event Handle that tells us the sound has finished being played.
				   // This is a very efficient way to put the program to sleep
				   // while the sound card is processing the sound buffer

	vector<char> m_data; // Sound data buffer
};