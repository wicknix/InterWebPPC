/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef nsJARChannel_h__
#define nsJARChannel_h__

#include "mozilla/net/MemoryDownloader.h"
#include "nsIJARChannel.h"
#include "nsIJARURI.h"
#include "nsIInputStreamPump.h"
#include "InterceptedJARChannel.h"
#include "nsIInterfaceRequestor.h"
#include "nsIProgressEventSink.h"
#include "nsIStreamListener.h"
#include "nsIRemoteOpenFileListener.h"
#include "nsIZipReader.h"
#include "nsILoadGroup.h"
#include "nsILoadInfo.h"
#include "nsIThreadRetargetableRequest.h"
#include "nsIThreadRetargetableStreamListener.h"
#include "nsHashPropertyBag.h"
#include "nsIFile.h"
#include "nsIURI.h"
#include "nsCOMPtr.h"
#include "nsString.h"
#include "mozilla/Logging.h"

class nsJARInputThunk;
class nsInputStreamPump;

namespace mozilla {
namespace net {
  class InterceptedJARChannel;
} // namespace net
} // namespace mozilla

//-----------------------------------------------------------------------------

class nsJARChannel final : public nsIJARChannel
                         , public mozilla::net::MemoryDownloader::IObserver
                         , public nsIStreamListener
                         , public nsIRemoteOpenFileListener
                         , public nsIThreadRetargetableRequest
                         , public nsIThreadRetargetableStreamListener
                         , public nsHashPropertyBag
{
public:
    NS_DECL_ISUPPORTS_INHERITED
    NS_DECL_NSIREQUEST
    NS_DECL_NSICHANNEL
    NS_DECL_NSIJARCHANNEL
    NS_DECL_NSIREQUESTOBSERVER
    NS_DECL_NSISTREAMLISTENER
    NS_DECL_NSIREMOTEOPENFILELISTENER
    NS_DECL_NSITHREADRETARGETABLEREQUEST
    NS_DECL_NSITHREADRETARGETABLESTREAMLISTENER

    nsJARChannel();

    nsresult Init(nsIURI *uri);

    nsresult OverrideSecurityInfo(nsISupports* aSecurityInfo);
    void OverrideURI(nsIURI* aRedirectedURI);

private:
    virtual ~nsJARChannel();

    nsresult CreateJarInput(nsIZipReaderCache *, nsJARInputThunk **);
    nsresult LookupFile(bool aAllowAsync);
    nsresult OpenLocalFile();
    void NotifyError(nsresult aError);
    void FireOnProgress(uint64_t aProgress);
    nsresult SetRemoteNSPRFileDesc(PRFileDesc *fd);
    virtual void OnDownloadComplete(mozilla::net::MemoryDownloader* aDownloader,
                                    nsIRequest* aRequest,
                                    nsISupports* aCtxt,
                                    nsresult aStatus,
                                    mozilla::net::MemoryDownloader::Data aData)
        override;

    bool BypassServiceWorker() const;

    nsresult SetContentTypeGuess();
    void SetOpened();

    // Returns true if this channel should intercept the network request and
    // prepare for a possible synthesized response instead.
    bool ShouldIntercept();

    nsresult ContinueAsyncOpen();
    void FinishAsyncOpen();

    // Discard the prior interception and continue with the original network
    // request.
    void ResetInterception();
    // Override this channel's pending response with a synthesized one. The
    // content will be asynchronously read from the pump.
    void OverrideWithSynthesizedResponse(nsIInputStream* aSynthesizedInput,
                                         const nsACString& aContentType);

    nsCString                       mSpec;

    bool                            mOpened;

    nsCOMPtr<nsIJARURI>             mJarURI;
    nsCOMPtr<nsIURI>                mOriginalURI;
    nsCOMPtr<nsIURI>                mAppURI;
    nsCOMPtr<nsISupports>           mOwner;
    nsCOMPtr<nsILoadInfo>           mLoadInfo;
    nsCOMPtr<nsIInterfaceRequestor> mCallbacks;
    nsCOMPtr<nsISupports>           mSecurityInfo;
    nsCOMPtr<nsIProgressEventSink>  mProgressSink;
    nsCOMPtr<nsILoadGroup>          mLoadGroup;
    nsCOMPtr<nsIStreamListener>     mListener;
    nsCOMPtr<nsISupports>           mListenerContext;
    nsCString                       mContentType;
    nsCString                       mContentCharset;
    nsCString                       mContentDispositionHeader;
    /* mContentDisposition is uninitialized if mContentDispositionHeader is
     * empty */
    uint32_t                        mContentDisposition;
    int64_t                         mContentLength;
    uint32_t                        mLoadFlags;
    nsresult                        mStatus;
    bool                            mIsPending;
    bool                            mIsUnsafe;
    bool                            mOpeningRemote;

    mozilla::net::MemoryDownloader::Data mTempMem;
    nsCOMPtr<nsIInputStreamPump>    mPump;
    // mRequest is only non-null during OnStartRequest, so we'll have a pointer
    // to the request if we get called back via RetargetDeliveryTo.
    nsCOMPtr<nsIRequest>            mRequest;
    nsCOMPtr<nsIFile>               mJarFile;
    nsCOMPtr<nsIURI>                mJarBaseURI;
    nsCString                       mJarEntry;
    nsCString                       mInnerJarEntry;

    RefPtr<nsInputStreamPump>     mSynthesizedResponsePump;
    int64_t                         mSynthesizedStreamLength;

    // True if this channel should not download any remote files.
    bool                            mBlockRemoteFiles;
    friend class mozilla::net::InterceptedJARChannel;
};

#endif // nsJARChannel_h__
