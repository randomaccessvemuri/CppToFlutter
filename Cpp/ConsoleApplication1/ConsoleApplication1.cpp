#include <zmq.hpp>
#include <iostream>
#include <json/json.h>
#include <optional>
#include <omp.h>


#define IMAGE_WIDTH 1000
#define IMAGE_HEIGHT 1000


int numMandelBrotBounces(double x, double y, int maxIterations) {
	double x0 = x;
	double y0 = y;
	int iteration = 0;
    while (x * x + y * y <= 4 && iteration < maxIterations) {
		double xtemp = x * x - y * y + x0;
		y = 2 * x * y + y0;
		x = xtemp;
		iteration++;
	}
	return iteration;
}

int* mandelBrotRGBAGenerator(int height, int width, double x, double y) {
    	int* image = new int[height * width * 4];
	int maxIterations = 1000;
	double x0 = x-2.5;
	double y0 = y-1;
	double x1 = x+1;
	double y1 = y+1;
	double xStep = (x1 - x0) / width;
	double yStep = (y1 - y0) / height;
	int index = 0;

	int dCounter = 0;
	
//#pragma omp parallel for collapse(2) schedule(dynamic, 1) num_threads(4) shared(image) 
	for (int y = 0; y < height; y++) {
		double yCoord = y0 + y * yStep;
		for (int x = 0; x < width; x++) {
			double xCoord = x0 + x * xStep;
			int iterations = numMandelBrotBounces(xCoord, yCoord, maxIterations);
			int r = iterations % 256;
			int g = iterations % 256;
			int b = iterations % 256;
			int a = 255;
			image[index] = r;
			image[index + 1] = g;
			image[index + 2] = b;
			image[index + 3] = a;
			index += 4;
			dCounter++;
		}
	}
	return image;

}

std::optional<std::pair<double, double>> getCoordsFromString(std::string strIn) {
	//We have string formatted as: {x: $x, y:$y}
	//We want to return a pair of doubles (x,y)
	std::pair<double, double> coords;
	
	//use the json library
	Json::Value root;
	Json::Reader reader;
	bool parsingSuccessful = reader.parse(strIn, root);

	coords.first = root["x"].asDouble();
	coords.second = root["y"].asDouble();

	return coords;		
}




int main() {
	

    zmq::context_t context(1);
    zmq::socket_t socket(context, zmq::socket_type::pub);

    socket.bind("tcp://127.0.0.1:5555");

	int counter = 0;

    while (true) {
		//Sending counter value

		double x = -0.5;
		double y = 0;

		//Now we generate the image
		int* image = mandelBrotRGBAGenerator(IMAGE_WIDTH, IMAGE_HEIGHT, x, y);

		//Now we send the image
		//We send the image as a json object
		Json::Value root;

		root["code"] = 200;
		root["width"] = IMAGE_HEIGHT;
		root["height"] = IMAGE_WIDTH;
		root["image"] = Json::Value(Json::arrayValue);

		for (int i = 0; i < IMAGE_HEIGHT * IMAGE_WIDTH * 4; i += 4) {
			root["image"].append(image[i]);
		}




		zmq::send_result_t stat = socket.send(zmq::buffer(root.toStyledString()), zmq::send_flags::dontwait);
		counter++;

		std::cout << "RECEIVED STATUS: " << stat.value();

		delete[] image;

    }

    return 0;
}





