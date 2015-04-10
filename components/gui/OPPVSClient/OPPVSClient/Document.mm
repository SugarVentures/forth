//
//  Document.m
//  OPPVSClient
//
//  Created by Cao Minh Trang on 3/27/15.
//  Copyright (c) 2015 Cao Minh Trang. All rights reserved.
//

#import "Document.h"
#include <errno.h>

oppvs::Stream* oppvsStream;

@interface Document ()
{
@private
    NSViewController* viewController;
}
@end


@implementation Document

const int expireTime = 2;    //5 seconds
const int maxTimeout = 3;
int numTimeOut = 0;
oppvs::SRTPSocket clientSocket;

- (instancetype)init {
    self = [super init];
    if (self) {
        
    }
    return self;
}

- (void)initReceiver: (NSString*)server withPort: (NSInteger)port
{
    oppvs::IPAddress localIpAddr;
    
    oppvs::SocketAddress localSocketAddr; //Socket for signaling
    localSocketAddr.setIP(localIpAddr);
    localSocketAddr.setPort(32120);
    
    struct in_addr in;
    inet_pton(AF_INET, [server UTF8String], &in);
    //inet_pton(AF_INET, "127.0.0.1", &in);
    oppvs::IPAddress serverIpAddr = oppvs::IPAddress(in);
    oppvs::SocketAddress serverSocketAddr;  //Address of signaling server
    serverSocketAddr.setIP(serverIpAddr);
    serverSocketAddr.setPort((int)port);
    
    oppvs::ClientSocket signalSocket;
    int sid = signalSocket.Create(AF_INET, SOCK_STREAM, 0);
    if (sid < 0)
    {
        NSLog(@"Create signaling socket failed\n");
        return;
    }
    if (signalSocket.Bind(localSocketAddr) < 0)
    {
        NSLog(@"Binding socket error\n");
        return;
    }
    if (signalSocket.Connect(serverSocketAddr) < 0)
    {
        NSLog(@"Cannot connect signaling server\n");
        printf("Connect error %s\n", strerror(errno));
        return;
    }
    signalSocket.Close();
    
    char key[oppvs::MAX_SRTP_KEY_LENGTH];
 
    clientSocket.initSRTPLib();
    clientSocket.setPolicy(key);
    
    localSocketAddr.setPort(33433);
    if (clientSocket.Create(AF_INET, SOCK_DGRAM, IPPROTO_UDP) < 0)
    {
        NSLog(@"Cant create srtp socket \n");
        return;
    }
    if (clientSocket.Bind(localSocketAddr) < 0)
    {
        NSLog(@"Binding error\n");
        return;
    }
    if (clientSocket.Connect(serverSocketAddr) < 0)
    {
        NSLog(@"Cannot connect streaming server\n");
        return;
    }
    clientSocket.initReceiver();
    
    
    //dispatch_queue_t queue = dispatch_queue_create("oppvs.receive.queue", DISPATCH_QUEUE_SERIAL);
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        [self runStreaming: clientSocket];
    });

    /*dispatch_group_t myGroup = dispatch_group_create();
    dispatch_group_async(myGroup, queue, ^{
        [self runStreaming: clientSocket];
    });
    dispatch_group_notify(myGroup, queue, ^{
        NSLog(@"Work is done!");
    });*/
}

- (void) runStreaming: (oppvs::SRTPSocket) socket
{
    char buffer[OPPVS_NETWORK_PACKET_LENGTH];
    int len = OPPVS_NETWORK_PACKET_LENGTH;
    oppvs::PixelBuffer pf;
    
    bool isNextFrame = false;
    uint32_t curPos = 0;

    ViewController *view = (ViewController*)viewController;
    bool interrupt = false;
    int count = 0;
    uint32_t oldNBytes = 0;
    socket.setReceiveTimeOut(expireTime);
    bool waitNextFrame = false;
    
    while (!interrupt)
    {
        memset(buffer, 0, sizeof(buffer));
        int recvLen = socket.RecvFrom(buffer, &len, &isNextFrame);
        if (recvLen > -1)
        {
            if (isNextFrame)
            {
                //NSLog(@"No of received pieces: %d\n", count);
                if (!waitNextFrame)
                {
                    [view renderFrame: &pf];
                }
                else
                    waitNextFrame = false;

                //Convert message to pixel buffer
                memcpy(&pf.width[0], buffer, 2);
                memcpy(&pf.height[0], buffer + 2, 2);
                memcpy(&pf.stride[0], buffer + 4, 2);
                memcpy(&pf.flip, buffer + 6, 1);
                uint32_t nb = pf.stride[0] * pf.height[0];
                pf.nbytes = nb;
                if (nb <= 0)
                    continue;
                if (oldNBytes != nb && nb > 0)
                {
                    delete pf.plane[0];
                    pf.plane[0] = new uint8_t[nb];
                    oldNBytes = nb;
                }
                memcpy(pf.plane[0], buffer+7, recvLen - 7);
                curPos = recvLen - 7;
                count = 1;
                
            }
            else
            {
                if (!waitNextFrame)
                {
                    //NSLog(@"Data: %d %d\n", len, recvLen);
                    memcpy(pf.plane[0] + curPos, buffer, recvLen);
                    curPos += recvLen;
                    //NSLog(@"No of received pieces: %d\n", count);
                    count++;
                }
            }
        }
        else
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                if (numTimeOut > maxTimeout)
                {
                    interrupt = true;
                }
                numTimeOut++;
            }
            else
            {
                waitNextFrame = true;
                NSLog(@"Error. Discard frame\n");
            }
        }

    }
    
    socket.releaseReceiver();
    socket.Close();

}


- (void)windowControllerDidLoadNib:(NSWindowController *)aController {
    [super windowControllerDidLoadNib:aController];
    // Add any code here that needs to be executed once the windowController has loaded the document's window.
}

+ (BOOL)autosavesInPlace {
    return YES;
}

- (void)makeWindowControllers {
    // Override to return the Storyboard file name of the document.
    [self addWindowController:[[NSStoryboard storyboardWithName:@"Main" bundle:nil] instantiateControllerWithIdentifier:@"Document Window Controller"]];
    viewController = self.windowForSheet.contentViewController;
    //[self initReceiver];
}

- (NSData *)dataOfType:(NSString *)typeName error:(NSError **)outError {
    // Insert code here to write your document to data of the specified type. If outError != NULL, ensure that you create and set an appropriate error when returning nil.
    // You can also choose to override -fileWrapperOfType:error:, -writeToURL:ofType:error:, or -writeToURL:ofType:forSaveOperation:originalContentsURL:error: instead.
    [NSException raise:@"UnimplementedMethod" format:@"%@ is unimplemented", NSStringFromSelector(_cmd)];
    return nil;
}

- (BOOL)readFromData:(NSData *)data ofType:(NSString *)typeName error:(NSError **)outError {
    // Insert code here to read your document from the given data of the specified type. If outError != NULL, ensure that you create and set an appropriate error when returning NO.
    // You can also choose to override -readFromFileWrapper:ofType:error: or -readFromURL:ofType:error: instead.
    // If you override either of these, you should also override -isEntireFileLoaded to return NO if the contents are lazily loaded.
    [NSException raise:@"UnimplementedMethod" format:@"%@ is unimplemented", NSStringFromSelector(_cmd)];
    return YES;
}

@end
