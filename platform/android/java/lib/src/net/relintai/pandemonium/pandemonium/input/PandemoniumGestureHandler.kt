/*************************************************************************/
/*  PandemoniumGestureHandler.kt                                               */
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

package net.relintai.pandemonium.pandemonium.input

import android.os.Build
import android.view.GestureDetector.SimpleOnGestureListener
import android.view.InputDevice
import android.view.MotionEvent
import android.view.ScaleGestureDetector
import android.view.ScaleGestureDetector.OnScaleGestureListener
import net.relintai.pandemonium.pandemonium.PandemoniumLib

/**
 * Handles regular and scale gesture input related events for the [PandemoniumView] view.
 *
 * @See https://developer.android.com/reference/android/view/GestureDetector.SimpleOnGestureListener
 * @See https://developer.android.com/reference/android/view/ScaleGestureDetector.OnScaleGestureListener
 */
internal class PandemoniumGestureHandler : SimpleOnGestureListener(), OnScaleGestureListener {

	companion object {
		private val TAG = PandemoniumGestureHandler::class.java.simpleName
	}

	/**
	 * Enable pan and scale gestures
	 */
	var panningAndScalingEnabled = false

	private var nextDownIsDoubleTap = false
	private var dragInProgress = false
	private var scaleInProgress = false
	private var contextClickInProgress = false
	private var pointerCaptureInProgress = false

	override fun onDown(event: MotionEvent): Boolean {
		PandemoniumInputHandler.handleMotionEvent(event.source, MotionEvent.ACTION_DOWN, event.buttonState, event.x, event.y, nextDownIsDoubleTap)
		nextDownIsDoubleTap = false
		return true
	}

	override fun onSingleTapUp(event: MotionEvent): Boolean {
		PandemoniumInputHandler.handleMotionEvent(event)
		return true
	}

	override fun onLongPress(event: MotionEvent) {
		contextClickRouter(event)
	}

	private fun contextClickRouter(event: MotionEvent) {
		if (scaleInProgress || nextDownIsDoubleTap) {
			return
		}

		// Cancel the previous down event
		PandemoniumInputHandler.handleMotionEvent(
			event.source,
			MotionEvent.ACTION_CANCEL,
			event.buttonState,
			event.x,
			event.y
		)

		// Turn a context click into a single tap right mouse button click.
		PandemoniumInputHandler.handleMouseEvent(
			MotionEvent.ACTION_DOWN,
			MotionEvent.BUTTON_SECONDARY,
			event.x,
			event.y
		)
		contextClickInProgress = true
	}

	fun onPointerCaptureChange(hasCapture: Boolean) {
		if (pointerCaptureInProgress == hasCapture) {
			return
		}

		if (!hasCapture) {
			// Dispatch a mouse relative ACTION_UP event to signal the end of the capture
			PandemoniumInputHandler.handleMouseEvent(
				MotionEvent.ACTION_UP,
				0,
				0f,
				0f,
				0f,
				0f,
				false,
				true
			)
		}
		pointerCaptureInProgress = hasCapture
	}

	fun onMotionEvent(event: MotionEvent): Boolean {
		return when (event.actionMasked) {
			MotionEvent.ACTION_UP, MotionEvent.ACTION_CANCEL, MotionEvent.ACTION_BUTTON_RELEASE -> {
				onActionUp(event)
			}
			MotionEvent.ACTION_MOVE -> {
				onActionMove(event)
			}
			else -> false
		}
	}

	private fun onActionUp(event: MotionEvent): Boolean {
		if (event.actionMasked == MotionEvent.ACTION_CANCEL && pointerCaptureInProgress) {
			// Don't dispatch the ACTION_CANCEL while a capture is in progress
			return true
		}

		val sourceMouseRelative = if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
			event.isFromSource(InputDevice.SOURCE_MOUSE_RELATIVE)
		} else {
			false
		}

		if (pointerCaptureInProgress || dragInProgress || contextClickInProgress) {
			if (contextClickInProgress || PandemoniumInputHandler.isMouseEvent(event)) {
				// This may be an ACTION_BUTTON_RELEASE event which we don't handle,
				// so we convert it to an ACTION_UP event.
				PandemoniumInputHandler.handleMouseEvent(
					MotionEvent.ACTION_UP,
					event.buttonState,
					event.x,
					event.y,
					0f,
					0f,
					false,
					sourceMouseRelative
				)
			} else {
				PandemoniumInputHandler.handleTouchEvent(event)
			}

			pointerCaptureInProgress = false
			dragInProgress = false
			contextClickInProgress = false
			return true
		}

		return false
	}

	private fun onActionMove(event: MotionEvent): Boolean {
		if (contextClickInProgress) {
			val sourceMouseRelative = if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
				event.isFromSource(InputDevice.SOURCE_MOUSE_RELATIVE)
			} else {
				false
			}

			PandemoniumInputHandler.handleMouseEvent(
				event.actionMasked,
				MotionEvent.BUTTON_SECONDARY,
				event.x,
				event.y,
				0f,
				0f,
				false,
				sourceMouseRelative
			)
			return true
		}
		return false
	}

	override fun onDoubleTapEvent(event: MotionEvent): Boolean {
		if (event.actionMasked == MotionEvent.ACTION_UP) {
			nextDownIsDoubleTap = false
			PandemoniumInputHandler.handleMotionEvent(event)
		}

		return true
	}

	override fun onDoubleTap(event: MotionEvent): Boolean {
		nextDownIsDoubleTap = true
		return true
	}

	override fun onScroll(
		originEvent: MotionEvent,
		terminusEvent: MotionEvent,
		distanceX: Float,
		distanceY: Float
	): Boolean {
		if (scaleInProgress) {
			if (dragInProgress) {
				// Cancel the drag
				PandemoniumInputHandler.handleMotionEvent(
					originEvent.source,
					MotionEvent.ACTION_CANCEL,
					originEvent.buttonState,
					originEvent.x,
					originEvent.y
				)
				dragInProgress = false
			}
		}

		val x = terminusEvent.x
		val y = terminusEvent.y
		if (terminusEvent.pointerCount >= 2 && panningAndScalingEnabled && !pointerCaptureInProgress) {
			PandemoniumLib.pan(x, y, distanceX / 5f, distanceY / 5f)
		} else if (!scaleInProgress) {
			PandemoniumInputHandler.handleMotionEvent(terminusEvent)
		}

		return true
	}

	override fun onScale(detector: ScaleGestureDetector): Boolean {
		if (!panningAndScalingEnabled || pointerCaptureInProgress) {
			return false
		}

		if (detector.scaleFactor >= 0.8f && detector.scaleFactor != 1f && detector.scaleFactor <= 1.2f) {
			PandemoniumLib.magnify(
				detector.focusX,
				detector.focusY,
				detector.scaleFactor
			)
		}
		return true
	}

	override fun onScaleBegin(detector: ScaleGestureDetector): Boolean {
		if (!panningAndScalingEnabled || pointerCaptureInProgress) {
			return false
		}
		scaleInProgress = true
		return true
	}

	override fun onScaleEnd(detector: ScaleGestureDetector) {
		scaleInProgress = false
	}
}