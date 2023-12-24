import 'dart:async';
import 'dart:io';
import 'package:dartzmq/dartzmq.dart';
import 'package:flutter/material.dart';
import 'package:provider/provider.dart';
import 'package:image/image.dart' as img;
import 'dart:convert';

Image imageFromZMessage(ZMessage message) {
  final data = message.toList(growable: false);
  ZFrame dataEncoded = data[0];

  //Convert the ZFrame to a byte array
  final bytes = dataEncoded.payload.toList(growable: false);

  //Convert the array to a json string
  final json = String.fromCharCodes(bytes);

  //Convert the json string to a map
  final map = jsonDecode(json) as Map<String, dynamic>;

  //Convert the map to a byte array
  final imageBytes = map["image"].cast<int>();

  //Convert the byte array to an image
  final image = img.Image(width: map["width"], height: map["height"]);

  //Fill the image with the byte array

  for (int i = 4; i < imageBytes.length; i += 4) {
    image.setPixelRgba(i % image.width, i ~/ image.width, imageBytes[i - 3],
        imageBytes[i - 2], imageBytes[i - 1], imageBytes[i]);
  }
  Image res = Image.memory(img.encodePng(image));
  return res;
}

void main() {
  runApp(MyApp());
}

class UserProvider extends ChangeNotifier {
  late Image _data_Received;
  late StreamSubscription<ZMessage> _subscription;
  late ZSocket _socket;
  late ZContext _context;

  UserProvider() {
    print("Surveyor Constructor");
    //Dummy image
    _data_Received = Image.network(
        "https://i.pinimg.com/736x/7b/88/fc/7b88fcba7c38856f93cf8d8b729b4de8.jpg");
    _context = ZContext();
    _socket = _context.createSocket(SocketType.sub);
    _socket.connect("tcp://localhost:5555");
    _socket.subscribe("");
    _subscription = _socket.messages.listen((event) {
      _data_Received = imageFromZMessage(event);
      notifyListeners();
    });
  }

  void buttonPressed() {
    print("Button Pressed");
    _socket.sendString("Hello", nowait: true);
  }
}

class MyApp extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return ChangeNotifierProvider(
      create: (context) => UserProvider(),
      child: Consumer<UserProvider>(
        builder: (context, value, child) => MaterialApp(
          home: Scaffold(
            appBar: AppBar(
              title: Text("DartzMQ"),
            ),
            body: Center(
              child: Column(
                mainAxisAlignment: MainAxisAlignment.center,
                children: [
                  Text("Data Received"),
                  Container(
                    width: WidgetsBinding.instance.platformDispatcher.views
                        .first.physicalGeometry.width,
                    height: WidgetsBinding.instance.platformDispatcher.views
                            .first.physicalGeometry.height /
                        2,
                    child: value._data_Received,
                  ),
                  ElevatedButton(
                    onPressed: () {
                      value.buttonPressed();
                    },
                    child: Text("Send"),
                  ),
                ],
              ),
            ),
          ),
        ),
      ),
    );
  }
}
