#include <ppltasks.h>
#include <sstream>
...
using namespace Windows::Foundation;
using namespace Windows::Storage::Streams;
using namespace Windows::UI::Core;
using namespace Windows::UI::Xaml::Navigation;
...
private:
    Windows::Networking::Sockets::DatagramSocket^ clientDatagramSocket;
    Windows::Networking::Sockets::DatagramSocket^ serverDatagramSocket;

protected:
    virtual void OnNavigatedTo(NavigationEventArgs^ e) override
    {
        this->StartServer();
        this->StartClient();
    }

private:
    void StartServer()
    {
        try
        {
            this->serverDatagramSocket = ref new Windows::Networking::Sockets::DatagramSocket();

            // The ConnectionReceived event is raised when connections are received.
            this->serverDatagramSocket->MessageReceived += ref new TypedEventHandler<Windows::Networking::Sockets::DatagramSocket^, Windows::Networking::Sockets::DatagramSocketMessageReceivedEventArgs^>(this, &DatagramSocketPage::ServerDatagramSocket_MessageReceived);

            this->serverListBox->Items->Append(L"server is about to bind...");

            // Start listening for incoming TCP connections on the specified port. You can specify any port that's not currently in use.
            Concurrency::create_task(this->serverDatagramSocket->BindServiceNameAsync("1337")).then(
                [=]
            {
                this->serverListBox->Items->Append(L"server is bound to port number 1337");
            });
        }
        catch (Platform::Exception^ ex)
        {
            Windows::Networking::Sockets::SocketErrorStatus webErrorStatus = Windows::Networking::Sockets::SocketError::GetStatus(ex->HResult);
            this->serverListBox->Items->Append(webErrorStatus.ToString() != L"Unknown" ? webErrorStatus.ToString() : ex->Message);
        }
    }

    void ServerDatagramSocket_MessageReceived(Windows::Networking::Sockets::DatagramSocket^ sender, Windows::Networking::Sockets::DatagramSocketMessageReceivedEventArgs^ args)
    {
        DataReader^ dataReader = args->GetDataReader();
        Platform::String^ request = dataReader->ReadString(dataReader->UnconsumedBufferLength);
        this->Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler(
            [=]
        {
            std::wstringstream wstringstream;
            wstringstream << L"server received the request: \"" << request->Data() << L"\"";
            this->serverListBox->Items->Append(ref new Platform::String(wstringstream.str().c_str()));
        }));

        // Echo the request back as the response.
        Concurrency::create_task(sender->GetOutputStreamAsync(args->RemoteAddress, "1336")).then(
            [=](IOutputStream^ outputStream)
        {
            auto dataWriter = ref new DataWriter(outputStream);
            dataWriter->WriteString(request);
            Concurrency::create_task(dataWriter->StoreAsync()).then(
                [=](unsigned int)
            {
                dataWriter->DetachStream();

                this->Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler(
                    [=]()
                {
                    std::wstringstream wstringstream;
                    wstringstream << L"server sent back the response: \"" << request->Data() << L"\"";
                    this->serverListBox->Items->Append(ref new Platform::String(wstringstream.str().c_str()));

                    delete this->serverDatagramSocket;
                    this->serverDatagramSocket = nullptr;

                    this->Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([=]() {this->serverListBox->Items->Append(L"server closed its socket"); }));
                }));
            });
        });
    }

    void StartClient()
    {
        try
        {
            // Create the DatagramSocket and establish a connection to the echo server.
            this->clientDatagramSocket = ref new Windows::Networking::Sockets::DatagramSocket();

            this->clientDatagramSocket->MessageReceived += ref new TypedEventHandler<Windows::Networking::Sockets::DatagramSocket^, Windows::Networking::Sockets::DatagramSocketMessageReceivedEventArgs^>(this, &DatagramSocketPage::ClientDatagramSocket_MessageReceived);

            // The server hostname that we will be establishing a connection to. In this example, the server and client are in the same process.
            auto hostName = ref new Windows::Networking::HostName(L"localhost");

            this->clientListBox->Items->Append(L"client is about to bind...");

            Concurrency::create_task(this->clientDatagramSocket->BindServiceNameAsync("1336")).then(
                [=]
            {
                this->clientListBox->Items->Append(L"client is bound to port number 1336");
            });

            // Send a request to the echo server.
            auto serverDatagramSocket = ref new Windows::Networking::Sockets::DatagramSocket();
            Concurrency::create_task(serverDatagramSocket->GetOutputStreamAsync(hostName, "1337")).then(
                [=](IOutputStream^ outputStream)
            {
                auto request = ref new Platform::String(L"Hello, World!");
                auto dataWriter = ref new DataWriter(outputStream);
                dataWriter->WriteString(request);
                Concurrency::create_task(dataWriter->StoreAsync()).then(
                    [=](unsigned int)
                {
                    dataWriter->DetachStream();
                    std::wstringstream wstringstream;
                    wstringstream << L"client sent the request: \"" << request->Data() << L"\"";
                    this->clientListBox->Items->Append(ref new Platform::String(wstringstream.str().c_str()));
                });

            });
        }
        catch (Platform::Exception^ ex)
        {
            Windows::Networking::Sockets::SocketErrorStatus webErrorStatus = Windows::Networking::Sockets::SocketError::GetStatus(ex->HResult);
            this->serverListBox->Items->Append(webErrorStatus.ToString() != L"Unknown" ? webErrorStatus.ToString() : ex->Message);
        }
    }

    void ClientDatagramSocket_MessageReceived(Windows::Networking::Sockets::DatagramSocket^ sender, Windows::Networking::Sockets::DatagramSocketMessageReceivedEventArgs^ args)
    {
        DataReader^ dataReader = args->GetDataReader();
        Platform::String^ response = dataReader->ReadString(dataReader->UnconsumedBufferLength);
        this->Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler(
            [=]
        {
            std::wstringstream wstringstream;
            wstringstream << L"client received the response: \"" << response->Data() << L"\"";
            this->clientListBox->Items->Append(ref new Platform::String(wstringstream.str().c_str()));
        }));

        delete this->clientDatagramSocket;
        this->clientDatagramSocket = nullptr;

        this->Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([=]() {this->clientListBox->Items->Append(L"client closed its socket"); }));
    }