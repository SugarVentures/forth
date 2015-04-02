//
//  Document.m
//  OPPVSClient
//
//  Created by Cao Minh Trang on 3/27/15.
//  Copyright (c) 2015 Cao Minh Trang. All rights reserved.
//

#import "Document.h"


oppvs::Stream* oppvsStream;

@interface Document ()
{
@private
    NSViewController* viewController;
}
@end


@implementation Document

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
        return;
    }
    signalSocket.Close();
    
    char key[oppvs::MAX_SRTP_KEY_LENGTH];
    oppvs::SRTPSocket clientSocket;
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
    
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        [self runStreaming: clientSocket];
    });
    
   
}

- (void) runStreaming: (oppvs::SRTPSocket) socket
{
    char buffer[OPPVS_NETWORK_PACKET_LENGTH];
    int len = OPPVS_NETWORK_PACKET_LENGTH;
    oppvs::PixelBuffer pf;
    pf.width[0] = 1280;
    pf.height[0] = 720;
    pf.nbytes = 3686400;
    pf.plane[0] = new uint8_t[pf.nbytes];
    memset(pf.plane[0], 0, pf.nbytes);
    
    bool isNextFrame = false;
    uint32_t curPos = 0;
    
    ViewController *view = (ViewController*)viewController;
    bool interrupt = false;
    int count = 0;
    while (!interrupt)
    {
        memset(buffer, 0, sizeof(buffer));
        int recvLen = socket.RecvFrom(buffer, &len, &isNextFrame);
        if (recvLen > -1)
        {

            if (isNextFrame)
            {
                //NSLog(@"No of received pieces: %d\n", count);
                [view renderFrame: &pf];
                    //interrupt = true;
                    //continue;
                memcpy(pf.plane[0], buffer, recvLen);
                curPos = recvLen;
                count = 1;
            }
            else
            {
                //NSLog(@"Data: %d %d\n", len, recvLen);
                memcpy(pf.plane[0] + curPos, buffer, recvLen);
                curPos += recvLen;
                //NSLog(@"No of received pieces: %d\n", count);
                count++;
                //if (count == 3600)
                //    [view renderFrame:&pf];
                
            }
        }
        else
            NSLog(@"Error\n");
    }
    
    //socket.releaseReceiver();
    //socket.Close();
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
