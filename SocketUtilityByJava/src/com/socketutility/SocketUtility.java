package com.socketutility;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.net.InetSocketAddress;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.SocketTimeoutException;

import com.socketutility.exception.CancelException;

public class SocketUtility {
    private Socket socket = null;
    private ServerSocket server = null;

    private DataInputStream in;
    private DataOutputStream out;

    static private final int STREAM_BUFFER_SIZE = 1000000;
    static private final int BUFFER_SIZE = 256;
    private int bufferedSize = 0;
    private byte[] buffer = new byte[BUFFER_SIZE];

    private boolean canceling = false;

    public SocketUtility() {
    }

    public void canceling() {
        canceling = true;
    }

    public boolean close() {
        boolean ret = true;
        if (socket != null) {
            try {
                socket.close();
                socket = null;
            } catch (IOException e) {
                e.printStackTrace();
                ret = false;
            }
        }
        if (server != null) {
            try {
                server.close();
                server = null;
            } catch (IOException e) {
                e.printStackTrace();
                ret = false;
            }
        }
        bufferedSize = 0;
        return ret;
    }

    public boolean flush() {
        try {
            out.flush();
        } catch (IOException e) {
            e.printStackTrace();
            return false;
        }

        return true;
    }

    public DataInputStream getIn() {
        return in;
    }

    public DataOutputStream getOut() {
        return out;
    }

    public boolean accept(String host, int port) throws CancelException {
        try {
            server = new ServerSocket(port);
            socket = null;
            int timeout = 1000; // milliseconds
            server.setSoTimeout(timeout);
            while (socket == null) {
                try {
                    socket = server.accept();
                } catch (SocketTimeoutException e) {
                    if (canceling) {
                        canceling = false;
                        throw new CancelException();
                    }
                }
            }
            
            in = new DataInputStream(new BufferedInputStream(socket.getInputStream(), STREAM_BUFFER_SIZE));
            out = new DataOutputStream(new BufferedOutputStream(socket.getOutputStream(), STREAM_BUFFER_SIZE));
            bufferedSize = 0;
        } catch (IOException e) {
            if (canceling) {
                canceling = false;
                throw new CancelException();
            }
            return false;
        }

        return true;
    }

    public boolean connect(String host, int port) {
        try {
            socket = new Socket();
            socket.connect(new InetSocketAddress(host, port));
            in = new DataInputStream(new BufferedInputStream(socket.getInputStream(), STREAM_BUFFER_SIZE));
            out = new DataOutputStream(new BufferedOutputStream(socket.getOutputStream(), STREAM_BUFFER_SIZE));
            bufferedSize = 0;
        } catch (IOException e) {
            return false;
        }

        return true;
    }

    public String gets() {
        StringBuilder s = new StringBuilder();
        try {
            while (true) {
                if (bufferedSize == 0) {
                    bufferedSize = in.read(buffer, 0, BUFFER_SIZE);
                    if (bufferedSize == 0) {
                        return null;
                    } else if (bufferedSize < 0) {
                        if (s.length() <= 0) {
                            return null;
                        } else {
                            return s.toString();
                        }
                    }
                } else if (bufferedSize < 0) {
                    return null;
                }

                StringBuilder tmp = new StringBuilder();
                boolean b_text_end = false;

                for (int i = 0; i <= bufferedSize; i++) {
                    if (i == bufferedSize) {
                        for (int j = 0; j < bufferedSize; j++) {
                            tmp.append((char) buffer[j]);
                        }
                        bufferedSize = 0;
                        break;
                    }
                    if (buffer[i] == '\0' || buffer[i] == '\n') {
                        for (int j = 0; j < i; j++) {
                            tmp.append((char) buffer[j]);
                        }
                        bufferedSize -= i + 1;
                        for (int j = 0; j < bufferedSize; j++) {
                            buffer[j] = buffer[j + i + 1];
                        }
                        b_text_end = true;
                        break;
                    }
                }
                s.append(tmp);
                if (b_text_end)
                    break;
            }
        } catch (IOException e) {
            e.printStackTrace();
            return null;
        }
        return s.toString();
    }

    public boolean print(String str) {
        try {
            out.write(str.getBytes());
            out.writeByte(0);
        } catch (IOException e) {
            e.printStackTrace();
            return false;
        }
        return true;
    }

    public boolean read(byte[] data, int dataSize) {
        try {
            int size = 0;
            if (bufferedSize > 0) {
                int copySize = (dataSize < bufferedSize ? dataSize : bufferedSize);
                for (int i = 0; i < copySize; i++) {
                    data[i] = buffer[i];
                }
                bufferedSize -= copySize;
                size += copySize;
                for (int i = 0; i < bufferedSize; i++) {
                    buffer[i] = buffer[copySize + i];
                }
            }

            while (true) {
                if (size < dataSize) {
                    int length = in.read(data, size, dataSize - size);
                    if (length < 0) {
                        return false;
                    }
                    size += length;
                } else {
                    return true;
                }
            }
        } catch (IOException e) {
            e.printStackTrace();
            return false;
        }
    }

    public boolean write(byte[] data) {
        try {
            out.write(data);
        } catch (IOException e) {
            e.printStackTrace();
            return false;
        }
        return true;
    }
}

