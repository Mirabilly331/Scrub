#include <iostream>
#include <exception>
#include <stdexcept>
#include <string>

#include <mpi.h>


namespace mirabella {

enum class MPIFileSegment_IOType { MPIFileSegment_IOType_READ, MPIFileSegment_IOType_WRITE };


class MPIFileSegment
{
public:
    MPIFileSegment(const std::string& filename, 
                   const int number_of_segments, 
                   const int segment_index, 
                   const int mode = MPI_MODE_RDWR,
                   const MPI_Offset size = 0);

    ~MPIFileSegment();

    int ReadBlockAt(const  MPI_Offset count, char* buffer, const MPI_Offset start);

    int WriteBlockAt(const  MPI_Offset count, char* buffer, const MPI_Offset start);

    off_t GetFileSize() const;

private:
    std::string     fs_filename;              	// The nme of the file in the filesystem
    MPI_File        fs_handle;                	// The MPI file handle of the opened file
    int             fs_mode;                  	// MPI file access mode

    int             fs_number_of_segments;       	// The total number of segmentsin which to divide the file
    int             fs_segment_index;         	// The segment that should be opened, 1 based

    MPI_Offset      fs_file_size;             	// The total size of the file in bytes
    MPI_Offset      fs_segment_start;         	// The offset in the file at which the segment starts
    MPI_Offset      fs_segment_end;           	// The point beyond which we cannot read or write in the file
    MPI_Offset      fs_segment_length;        	// Number of bytes in this segment

    int IO_switch(const  MPI_Offset count, char* buffer, const MPI_Offset start, MPIFileSegment_IOType type);
};


} // namespace mirabella
