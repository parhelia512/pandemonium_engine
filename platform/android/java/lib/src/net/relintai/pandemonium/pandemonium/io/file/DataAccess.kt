/*************************************************************************/
/*  DataAccess.kt                                                        */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2022 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2022 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

package net.relintai.pandemonium.pandemonium.io.file

import android.content.Context
import android.os.Build
import android.util.Log
import net.relintai.pandemonium.pandemonium.io.StorageScope
import java.io.IOException
import java.nio.ByteBuffer
import java.nio.channels.FileChannel
import kotlin.math.max

/**
 * Base class for file IO operations.
 *
 * Its derived instances provide concrete implementations to handle regular file access, as well
 * as file access through the media store API on versions of Android were scoped storage is enabled.
 */
internal abstract class DataAccess(private val filePath: String) {

	companion object {
		private val TAG = DataAccess::class.java.simpleName

		fun generateDataAccess(
			storageScope: StorageScope,
			context: Context,
			filePath: String,
			accessFlag: FileAccessFlags
		): DataAccess? {
			return when (storageScope) {
				StorageScope.APP -> FileData(filePath, accessFlag)

				StorageScope.SHARED -> if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) {
					MediaStoreData(context, filePath, accessFlag)
				} else {
					null
				}

				StorageScope.UNKNOWN -> null
			}
		}

		fun fileExists(storageScope: StorageScope, context: Context, path: String): Boolean {
			return when(storageScope) {
				StorageScope.APP -> FileData.fileExists(path)
				StorageScope.SHARED -> MediaStoreData.fileExists(context, path)
				StorageScope.UNKNOWN -> false
			}
		}

		fun fileLastModified(storageScope: StorageScope, context: Context, path: String): Long {
			return when(storageScope) {
				StorageScope.APP -> FileData.fileLastModified(path)
				StorageScope.SHARED -> MediaStoreData.fileLastModified(context, path)
				StorageScope.UNKNOWN -> 0L
			}
		}

		fun removeFile(storageScope: StorageScope, context: Context, path: String): Boolean {
			return when(storageScope) {
				StorageScope.APP -> FileData.delete(path)
				StorageScope.SHARED -> MediaStoreData.delete(context, path)
				StorageScope.UNKNOWN -> false
			}
		}

		fun renameFile(storageScope: StorageScope, context: Context, from: String, to: String): Boolean {
			return when(storageScope) {
				StorageScope.APP -> FileData.rename(from, to)
				StorageScope.SHARED -> MediaStoreData.rename(context, from, to)
				StorageScope.UNKNOWN -> false
			}
		}
	}

	protected abstract val fileChannel: FileChannel
	internal var endOfFile = false
		private set

	fun close() {
		try {
			fileChannel.close()
		} catch (e: IOException) {
			Log.w(TAG, "Exception when closing file $filePath.", e)
		}
	}

	fun flush() {
		try {
			fileChannel.force(false)
		} catch (e: IOException) {
			Log.w(TAG, "Exception when flushing file $filePath.", e)
		}
	}

	fun seek(position: Long) {
		try {
			fileChannel.position(position)
			if (position <= size()) {
				endOfFile = false
			}
		} catch (e: Exception) {
			Log.w(TAG, "Exception when seeking file $filePath.", e)
		}
	}

	fun seekFromEnd(positionFromEnd: Long) {
		val positionFromBeginning = max(0, size() - positionFromEnd)
		seek(positionFromBeginning)
	}

	fun position(): Long {
		return try {
			fileChannel.position()
		} catch (e: IOException) {
			Log.w(
				TAG,
				"Exception when retrieving position for file $filePath.",
				e
			)
			0L
		}
	}

	fun size() = try {
		fileChannel.size()
	} catch (e: IOException) {
		Log.w(TAG, "Exception when retrieving size for file $filePath.", e)
		0L
	}

	fun read(buffer: ByteBuffer): Int {
		return try {
			val readBytes = fileChannel.read(buffer)
			if (readBytes == -1) {
				endOfFile = true
				0
			} else {
				readBytes
			}
		} catch (e: IOException) {
			Log.w(TAG, "Exception while reading from file $filePath.", e)
			0
		}
	}

	fun write(buffer: ByteBuffer) {
		try {
			val writtenBytes = fileChannel.write(buffer)
			if (writtenBytes > 0) {
				endOfFile = false
			}
		} catch (e: IOException) {
			Log.w(TAG, "Exception while writing to file $filePath.", e)
		}
	}
}
