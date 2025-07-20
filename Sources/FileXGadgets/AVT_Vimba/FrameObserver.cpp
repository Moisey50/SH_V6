/*=============================================================================
  Copyright (C) 2012 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        FrameObserver.cpp

  Description: The frame observer that is used for notifications from VimbaCPP
               regarding the arrival of a newly acquired frame.

-------------------------------------------------------------------------------

  THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR IMPLIED
  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF TITLE,
  NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR  PURPOSE ARE
  DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
  AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=============================================================================*/

#include <stdafx.h>
#include <FrameObserver.h>

namespace AVT {
namespace VmbAPI {

void FrameObserver::FrameReceived( const FramePtr pFrame )
{
    bool bQueueDirectly = true;
    VmbFrameStatusType eReceiveStatus;

    if( m_CallBack 
      && (VmbErrorSuccess == pFrame->GetReceiveStatus( eReceiveStatus )) )
    {
      m_CallBack( pFrame , m_pClient ) ;      
    }

     m_pCamera->QueueFrame( pFrame );
}

// Returns the oldest frame that has not been picked up yet
// FramePtr FrameObserver::GetFrame()
// {
//     return NULL ;
// }
// 
// void FrameObserver::ClearFrameQueue()
// {
// //     // Lock the frame queue
// //     m_FramesMutex.Lock();
// //     // Clear the frame queue and release the memory
// //     std::queue<FramePtr> empty;
// //     std::swap( m_Frames, empty );
// //     // Unlock the frame queue
// //     m_FramesMutex.Unlock();
// }

}} // namespace AVT::VmbAPI::Examples
