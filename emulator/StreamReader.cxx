
/**
  Simulate retrieval of 512 byte chunks from a serial source
  @author  Rob Bairos
*/

#include "StreamReader.hxx"

#define FIELD_SIZE 2560 //round field to nearest 512 byte boundary
#define FRAME_SIZE 4096 // round to nearest 4K 

bool
StreamReader::open(const std::string& path)
{
	close();

	myFile = new std::ifstream(path, std::ios::binary);

	if (!myFile || !*myFile)
		close();

	myFile->seekg(0, std::ios::end);
	myFileSize = static_cast<size_t>(myFile->tellg());
	myFile->seekg(0, std::ios::beg);

	return myFile ? true:false;
}

void
StreamReader::close()
{
	delete myFile;
	myFile = nullptr;
}

void
StreamReader::swapField(bool index)
{
	if (index == true)
	{
		myVersion  = myBuffer1 + VERSION_DATA_OFFSET;
		myFrame  = myBuffer1 + FRAME_DATA_OFFSET;
		myAudio = myBuffer1 + AUDIO_DATA_OFFSET;
		myGraph = myBuffer1 + GRAPH_DATA_OFFSET;
		myTimecode= myBuffer1 + TIMECODE_DATA_OFFSET;
		myColor = myBuffer1 + COLOR_DATA_OFFSET;
	}
	else
	{
		myVersion  = myBuffer2 + VERSION_DATA_OFFSET;
		myFrame  = myBuffer2 + FRAME_DATA_OFFSET;
		myAudio = myBuffer2 + AUDIO_DATA_OFFSET;
		myGraph = myBuffer2 + GRAPH_DATA_OFFSET;
		myTimecode = myBuffer2 + TIMECODE_DATA_OFFSET;
		myColor = myBuffer2 + COLOR_DATA_OFFSET;
	}
}

void
StreamReader::blankPartialLines(bool index) 
{
	constexpr int colorSize = 192 * 5;
	if (index)
	{
		// top line
		myColor[0] = 0;
		myColor[1] = 0;
		myColor[2] = 0;
		myColor[3] = 0;
		myColor[4] = 0;
	}
	else
	{
		// bottom line
		myColor[colorSize - 5] = 0;
		myColor[colorSize - 4] = 0;
		myColor[colorSize - 3] = 0;
		myColor[colorSize - 2] = 0;
		myColor[colorSize - 1] = 0;
	}
}

bool
StreamReader::readField(uint32_t fnum, bool index)
{
	bool read = false;

	if (myFile)
	{
		size_t	offset = ((fnum + 0) * FRAME_SIZE);

		if (offset + FRAME_SIZE < myFileSize)
		{
			myFile->seekg((fnum + 0) * FRAME_SIZE);
			if (index == true)
				myFile->read((char*)myBuffer1, FIELD_SIZE);
			else
				myFile->read((char*)myBuffer2, FIELD_SIZE);

			read = true;
		}
	}

	return read;
}

