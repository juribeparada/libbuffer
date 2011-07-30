/* 
 * Copyright (C) 2009-2010 Chris McClelland
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *  
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file libbuffer.h
 *
 * The <b>Buffer</b> library provides functions to manipulate dynamic buffers of arbitrary binary
 * data, including reading and writing binary and Intel hex files.
 *
 * <code>struct Buffer b = {0,};</code><br/>
 * <code>bufInitialise(&b, 1024, 0x00, NULL);</code><br/>
 * <code>:</code><br/>
 * <code>bufDestroy(&b);</code>
 */
#ifndef LIBBUFFER_H
#define LIBBUFFER_H

#include <makestuff.h>

#ifdef __cplusplus
extern "C" {
#endif

	// ---------------------------------------------------------------------------------------------
	// Type declarations
	// ---------------------------------------------------------------------------------------------
	///@cond STRUCT
	/**
	 * The Buffer structure.
	 */	
	struct Buffer {
		uint8 *data;
		uint32 length;
		uint32 capacity;
		uint8 fill;
	};
	///@endcond

	/**
	 * @name Enumerations
	 * @{
	 */
	/**
	 * Return codes from the functions.
	 */	
	typedef enum {
		BUF_SUCCESS = 0,      ///< The operation completed successfully.
		BUF_NO_MEM,           ///< There was a memory allocation failure.
		BUF_FOPEN,            ///< There was a file open error.
		BUF_FSEEK,            ///< There was a binary file seek error.
		BUF_FEOF,             ///< There was an error testing for the end of a binary file.
		BUF_FERROR,           ///< Fewer bytes were read or written than expected.
		HEX_EMPTY_FILE,       ///< The I8HEX file was empty.
		HEX_JUNK_START_CODE,  ///< The first char of the I8HEX line was not ":".
		HEX_JUNK_BYTE_COUNT,  ///< The I8HEX byte count was invalid.
		HEX_JUNK_ADDR_MSB,	  ///< The I8HEX most-significant address byte was invalid.
		HEX_JUNK_ADDR_LSB,	  ///< The I8HEX least-significant address byte was invalid.
		HEX_JUNK_REC_TYPE,	  ///< The I8HEX record type was invalid.
		HEX_BAD_REC_TYPE,     ///< The I8HEX record type was unsupported.
		HEX_JUNK_DATA_BYTE,	  ///< One of the I8HEX data bytes was invalid.
		HEX_JUNK_CHECKSUM,	  ///< The I8HEX line checksum was invalid.
		HEX_BAD_CHECKSUM,	  ///< The I8HEX line checksum did not match the line data.
		HEX_CORRUPT_LINE,	  ///< The I8HEX line reconstruction did not match the original.
		HEX_MISSING_EOF,      ///< The I8HEX EOF record was missing.
		HEX_BAD_EXT_SEG       ///< The I8HEX EXT_SEG record was invalid.
	} BufferStatus;
	//@}

	// ---------------------------------------------------------------------------------------------
	// Core Operations
	// ---------------------------------------------------------------------------------------------
	/**
	 * @name Core Operations
	 * @{
	 */
	/**
	 * @brief Initialise a buffer ready for use.
	 *
	 * Initialise a buffer for use. Allocate an initial block of memory and fill it with a given
	 * byte value.
	 *
	 * @param self The buffer to initialise.
	 * @param initialSize The size of the initial buffer (i.e the threshold at which reallocation is
	 *            necessary).
	 * @param fill The byte value which is to be used as "background colour".
	 * @param error A pointer to a <code>char*</code> which will be set on exit to an allocated
	 *            error message if something goes wrong. Responsibility for this allocated memory
	 *            passes to the caller and must be freed with \c bufFreeError(). If \c error is
	 *            \c NULL, no allocation is done and no message is returned, but the return code
	 *            will still be valid.
	 * @returns
	 *     - \c BUF_SUCCESS if the operation completed successfully.
	 *     - \c BUF_NO_MEM if an allocation error occurred.
	 */
	DLLEXPORT(BufferStatus) bufInitialise(
		struct Buffer *self, uint32 initialSize, uint8 fill, const char **error
	) WARN_UNUSED_RESULT;

	/**
	 * @brief Destroy an already-initialised buffer.
	 *
	 * Free up any storage associated with the supplied buffer.
	 *
	 * @param self The buffer to destroy.
	 */
	DLLEXPORT(void) bufDestroy(
		struct Buffer *self
	);

	/**
	 * @brief Zero the length of the supplied buffer.
	 *
	 * Zero the length, but do not free the buffer's memory. Thus, the buffer may be reused without
	 * having to allocate new space.
	 *
	 * @param self The buffer to zero-length.
	 */
	DLLEXPORT(void) bufZeroLength(
		struct Buffer *self
	);

	/**
	 * @brief Append a single byte to the end of a buffer.
	 *
	 * Reallocate the buffer if necessary.
	 *
	 * @param self The buffer to append to.
	 * @param byte The byte to append.
	 * @param error A pointer to a <code>char*</code> which will be set on exit to an allocated
	 *            error message if something goes wrong. Responsibility for this allocated memory
	 *            passes to the caller and must be freed with \c bufFreeError(). If \c error is
	 *            \c NULL, no allocation is done and no message is returned, but the return code
	 *            will still be valid.
	 * @returns
	 *     - \c BUF_SUCCESS if the operation completed successfully.
	 *     - \c BUF_NO_MEM if an allocation error occurred.
	 */
	DLLEXPORT(BufferStatus) bufAppendByte(
		struct Buffer *self, uint8 byte, const char **error
	) WARN_UNUSED_RESULT;

	/**
	 * @brief Append a block of bytes to the end of a buffer.
	 *
	 * Reallocate the buffer if necessary.
	 *
	 * @param self The buffer to append to.
	 * @param ptr A pointer to the block of bytes to append.
	 * @param count The number of bytes to append.
	 * @param error A pointer to a <code>char*</code> which will be set on exit to an allocated
	 *            error message if something goes wrong. Responsibility for this allocated memory
	 *            passes to the caller and must be freed with \c bufFreeError(). If \c error is
	 *            \c NULL, no allocation is done and no message is returned, but the return code
	 *            will still be valid.
	 * @returns
	 *     - \c BUF_SUCCESS if the operation completed successfully.
	 *     - \c BUF_NO_MEM if an allocation error occurred.
	 */
	DLLEXPORT(BufferStatus) bufAppendBlock(
		struct Buffer *self, const uint8 *ptr, uint32 count, const char **error
	) WARN_UNUSED_RESULT;

	/**
	 * @brief Append a block of zero bytes to the end of a buffer.
	 *
	 * Reallocate the buffer if necessary.
	 *
	 * @param self The buffer to append to.
	 * @param count The number of zero bytes to append.
	 * @param ptr A pointer to a <code>uint8*</code> which will be set on exit to the new end of the
	 *            buffer's data. May be \c NULL.
	 * @param error A pointer to a <code>char*</code> which will be set on exit to an allocated
	 *            error message if something goes wrong. Responsibility for this allocated memory
	 *            passes to the caller and must be freed with \c bufFreeError(). If \c error is
	 *            \c NULL, no allocation is done and no message is returned, but the return code
	 *            will still be valid.
	 * @returns
	 *     - \c BUF_SUCCESS if the operation completed successfully.
	 *     - \c BUF_NO_MEM if an allocation error occurred.
	 */
	DLLEXPORT(BufferStatus) bufAppendZeros(
		struct Buffer *self, uint32 count, uint8 **ptr, const char **error
	) WARN_UNUSED_RESULT;

	/**
	 * @brief Append a block of identical bytes to the end of a buffer.
	 *
	 * Reallocate the buffer if necessary.
	 *
	 * @param self The buffer to append to.
	 * @param count The number of zero bytes to append.
	 * @param value The byte value to append.
	 * @param ptr A pointer to a <code>uint8*</code> which will be set on exit to the new end of the
	 *            buffer's data. May be \c NULL.
	 * @param error A pointer to a <code>char*</code> which will be set on exit to an allocated
	 *            error message if something goes wrong. Responsibility for this allocated memory
	 *            passes to the caller and must be freed with \c bufFreeError(). If \c error is
	 *            \c NULL, no allocation is done and no message is returned, but the return code
	 *            will still be valid.
	 * @returns
	 *     - \c BUF_SUCCESS if the operation completed successfully.
	 *     - \c BUF_NO_MEM if an allocation error occurred.
	 */
	DLLEXPORT(BufferStatus) bufAppendConst(
		struct Buffer *self, uint32 count, uint8 value, uint8 **ptr, const char **error
	) WARN_UNUSED_RESULT;

	/**
	 * @brief Copy a block of bytes into a buffer at a given offset.
	 *
	 * Reallocate the buffer if necessary. If the destination of the copy is off the end of the
	 * current buffer, the buffer will first be resized and the "hole" set to the fill byte.
	 *
	 * @param self The buffer to copy to.
	 * @param bufAddress The destination offset into the buffer.
	 * @param ptr A pointer to the source bytes.
	 * @param count The number of bytes to copy.
	 * @param error A pointer to a <code>char*</code> which will be set on exit to an allocated
	 *            error message if something goes wrong. Responsibility for this allocated memory
	 *            passes to the caller and must be freed with \c bufFreeError(). If \c error is
	 *            \c NULL, no allocation is done and no message is returned, but the return code
	 *            will still be valid.
	 * @returns
	 *     - \c BUF_SUCCESS if the operation completed successfully.
	 *     - \c BUF_NO_MEM if an allocation error occurred.
	 */
	DLLEXPORT(BufferStatus) bufCopyBlock(
		struct Buffer *self, uint32 bufAddress, const uint8 *ptr, uint32 count, const char **error
	) WARN_UNUSED_RESULT;

	/**
	 * @brief Set a specific block of bytes to a given value.
	 *
	 * Reallocate the buffer if necessary. If the block is off the end of the current buffer, the
	 * buffer will first be resized and the "hole" set to the fill byte.
	 *
	 * @param self The buffer to copy to.
	 * @param bufAddress The destination offset into the buffer.
	 * @param value The data byte to set the block to.
	 * @param count The number of bytes to set.
	 * @param error A pointer to a <code>char*</code> which will be set on exit to an allocated
	 *            error message if something goes wrong. Responsibility for this allocated memory
	 *            passes to the caller and must be freed with \c bufFreeError(). If \c error is
	 *            \c NULL, no allocation is done and no message is returned, but the return code
	 *            will still be valid.
	 * @returns
	 *     - \c BUF_SUCCESS if the operation completed successfully.
	 *     - \c BUF_NO_MEM if an allocation error occurred.
	 */
	DLLEXPORT(BufferStatus) bufSetBlock(
		struct Buffer *self, uint32 bufAddress, uint8 value, uint32 count, const char **error
	) WARN_UNUSED_RESULT;
	//@}

	// ---------------------------------------------------------------------------------------------
	// Binary I/O
	// ---------------------------------------------------------------------------------------------
	/**
	 * @name Binary I/O
	 * @{
	 */
	/**
	 * @brief Append a binary file to the end of a buffer.
	 *
	 * Reallocate if necessary.
	 *
	 * @param self The buffer to append to.
	 * @param fileName The binary file to append.
	 * @param error A pointer to a <code>char*</code> which will be set on exit to an allocated
	 *            error message if something goes wrong. Responsibility for this allocated memory
	 *            passes to the caller and must be freed with \c bufFreeError(). If \c error is
	 *            \c NULL, no allocation is done and no message is returned, but the return code
	 *            will still be valid.
	 * @returns
	 *     - \c BUF_SUCCESS if the operation completed successfully.
	 *     - \c BUF_NO_MEM if an allocation error occurred.
	 *     - \c BUF_FOPEN if the file could not be opened for reading.
	 *     - \c BUF_FSEEK if the file could not be fseek()'d.
	 *     - \c BUF_FEOF if the file could not be feof()'d.
	 *     - \c BUF_FERROR if the file could not be fread().
	 */
	DLLEXPORT(BufferStatus) bufAppendFromBinaryFile(
		struct Buffer *self, const char *fileName, const char **error
	) WARN_UNUSED_RESULT;

	/**
	 * @brief Write a block of raw data from a buffer to a binary file.
	 *
	 * @param self The buffer to save from.
	 * @param fileName The binary file to append.
	 * @param bufAddress The offset of the data block to be saved.
	 * @param count The number of bytes to save.
	 * @param error A pointer to a <code>char*</code> which will be set on exit to an allocated
	 *            error message if something goes wrong. Responsibility for this allocated memory
	 *            passes to the caller and must be freed with \c bufFreeError(). If \c error is
	 *            \c NULL, no allocation is done and no message is returned, but the return code
	 *            will still be valid.
	 * @returns
	 *     - \c BUF_SUCCESS if the operation completed successfully.
	 *     - \c BUF_FOPEN if the file could not be opened for writing.
	 *     - \c BUF_FERROR if the file could not be written to.
	 */
	DLLEXPORT(BufferStatus) bufWriteBinaryFile(
		const struct Buffer *self, const char *fileName, uint32 bufAddress, uint32 count,
		const char **error
	) WARN_UNUSED_RESULT;
	//@}

	// ---------------------------------------------------------------------------------------------
	// Intel Hex I/O
	// ---------------------------------------------------------------------------------------------
	/**
	 * @name Intel Hex I/O
	 * @{
	 */
	/**
	 * @brief Read an Intex hex (I8HEX) file into a buffer.
	 *
	 * Reallocate if necessary.
	 *
	 * @param destData The buffer to read data bytes into.
	 * @param destMask The buffer to read mask bytes into (may be /c NULL).
	 * @param fileName The I8HEX file to read.
	 * @param error A pointer to a <code>char*</code> which will be set on exit to an allocated
	 *            error message if something goes wrong. Responsibility for this allocated memory
	 *            passes to the caller and must be freed with \c bufFreeError(). If \c error is
	 *            \c NULL, no allocation is done and no message is returned, but the return code
	 *            will still be valid.
	 * @returns
	 *     - \c BUF_SUCCESS if the operation completed successfully.
	 *     - \c BUF_NO_MEM if an allocation error occurred.
	 *     - \c BUF_FOPEN if the file could not be opened for reading.
	 *     - \c HEX_EMPTY_FILE if the file was found to be empty.
	 *     - \c HEX_MISSING_EOF if the file was missing its I8HEX EOF record.
	 *     - \c HEX_JUNK_START_CODE if a line starts with something other than ":".
	 *     - \c HEX_JUNK_BYTE_COUNT if a byte count could not be parsed.
	 *     - \c HEX_JUNK_ADDR_MSB if an address MSB could not be parsed.
	 *     - \c HEX_JUNK_ADDR_LSB if an address LSB could not be parsed.
	 *     - \c HEX_JUNK_REC_TYPE if a record type could not be parsed.
	 *     - \c HEX_JUNK_DATA_BYTE if a data byte could not be parsed.
	 *     - \c HEX_JUNK_CHECKSUM if a checksum could not be parsed.
	 *     - \c HEX_BAD_CHECKSUM if a line checksum did not match the line.
	 *     - \c HEX_CORRUPT_LINE if a line's reconstruction did not match the original.
	 *     - \c HEX_BAD_EXT_SEG if an EXT_SEG record was invalid.
	 *     - \c HEX_BAD_REC_TYPE if an unsupported record type was encountered.
	 */
	DLLEXPORT(BufferStatus) bufReadFromIntelHexFile(
		struct Buffer *destData, struct Buffer *destMask, const char *fileName, const char **error
	) WARN_UNUSED_RESULT;

	/**
	 * @brief Write a buffer to an Intel hex (I8HEX) file.
	 *
	 * Writes out the content of a data buffer as I8HEX records, honouring an optional mask buffer.
	 * If the mask buffer is \c NULL, a mask buffer is derived. The derived mask can either be
	 * compressed or uncompressed. If compressed it just looks for sizeable runs of the fill byte in
	 * the data buffer. Deriving a mask with compression is dangerous because it assumes the
	 * consumer of your file will have initialised his or her buffer to a known value, which is not
	 * the case in general.
	 *
	 * @param sourceData The buffer to read data bytes from.
	 * @param sourceMask The buffer to read mask bytes from (may be \c NULL).
	 * @param fileName The I8HEX file to write.
	 * @param lineLength The I8HEX line length to use (usually 16 or 32 bytes).
	 * @param compress If sourceMask is \c NULL, whether the derived mask should be compressed.
	 * @param error A pointer to a <code>char*</code> which will be set on exit to an allocated
	 *            error message if something goes wrong. Responsibility for this allocated memory
	 *            passes to the caller and must be freed with \c bufFreeError(). If \c error is
	 *            \c NULL, no allocation is done and no message is returned, but the return code
	 *            will still be valid.
	 * @returns
	 *     - \c BUF_SUCCESS if the operation completed successfully.
	 *     - \c BUF_NO_MEM if an allocation error occurred during generation of the derived mask.
	 *     - \c BUF_FOPEN if the file could not be opened for writing.
	 */
	DLLEXPORT(BufferStatus) bufWriteToIntelHexFile(
		const struct Buffer *sourceData, const struct Buffer *sourceMask,
		const char *fileName, uint8 lineLength, bool compress, const char **error
	);
	//@}

	// ---------------------------------------------------------------------------------------------
	// Miscellaneous
	// ---------------------------------------------------------------------------------------------
	/**
	 * @name Miscellaneous
	 * @{
	 */
	/**
	 * @brief Free an error allocated when one of the other functions fails.
	 *
	 * @param err An error message previously allocated by one of the other library functions.
	 */
	DLLEXPORT(void) bufFreeError(const char *err);
	//@}

#ifdef __cplusplus
}
#endif

#endif
