#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>
#include <random>
#include <algorithm>

struct Vec3{
	double x, y, z;
	Vec3(double x = 0, double y = 0, double z = 0) : x(x), y(y), z(z) {}
	Vec3 operator+(const Vec3& v) const { return {x+v.x, y+v.y, z+v.z};}
	Vec3 operator-(const Vec3& v) const { return {x-v.x, y-v.y, z-v.z};}
	Vec3 operator*(const Vec3& v) const {return {x*v.x, y*v.y, z*v.z};}
	Vec3 operator*(double t) const {return {t*x, t*y, t*z};}
	Vec3 operator/(double t) const {return {x/t, y/t, z/t};}
	Vec3 &operator+=(const Vec3& v) {x+=v.x, y+=v.y, z+=v.z; return *this;}
	double dot(const Vec3& v) const {return { x*v.x+  y*v.y+ z*v.z};}
	
	Vec3 cross(const Vec3& v) const {
        return { y*v.z - z*v.y, z*v.x - x*v.z, x*v.y - y*v.x };
    }

	Vec3 normalize(){
		double len = std::sqrt(dot(*this));
		return len < 1e-8 ? *this:*this/len;
	}
};


struct Ray{
	Vec3 dir, origin;
	Ray(Vec3 og, Vec3 dir) : origin(og), dir(dir.normalize()){} 
	
};

struct Sphere{
	Vec3 center;
	double radius;
	Vec3 emission;
	Vec3 color;

	Sphere(Vec3 c, double r, Vec3 e, Vec3 col) :
		center(c), radius(r), emission(e), color(col) {}	
};

std::vector<Sphere> spheres = {
    Sphere(Vec3(0, -100.5, -1), 100,   Vec3(0,0,0),   Vec3(0.8,0.8,0.8)), // big floor (gray)
    Sphere(Vec3(0, 0, -1),      0.5,   Vec3(0,0,0),   Vec3(0.8,0.2,0.2)), // red ball
    Sphere(Vec3(1, 0, -1),      0.5,   Vec3(0,0,0),   Vec3(0.2,0.8,0.2)), // green ball
    Sphere(Vec3(-1, 0, -1),     0.5,   Vec3(0,0,0),   Vec3(0.2,0.2,0.8)), // blue ball
    Sphere(Vec3(0, -1.5, -1),   0.75,   Vec3(13,12,12), Vec3(0.6,0.6,0.6))       // bright white light
};

std::mt19937 rng(std::random_device{}());                  //random gen. 
std::uniform_real_distribution<double> uni(0.0, 1.0);        

double random_double() { return uni(rng); }                // mental note: make sure to replace it farvad branchless method

Vec3 random_cosine_direction(const Vec3& normal){

	double r1 = random_double();
	double r2 = random_double();

	double phi = 2*3.1415926535*r1;
	double cos_theta = std::sqrt(1 - r2);
	double sin_theta = std::sqrt(r2);

	Vec3 u = (std::abs(normal.x) > 0.1) ? Vec3(0, 1, 0) : Vec3(1, 0, 0).cross(normal).normalize();
	Vec3 v = normal.cross(u);

	return (u* (std::cos(phi)*sin_theta) + 
			v* (std::sin(phi)*cos_theta) +
			normal*cos_theta).normalize();
}

bool intersect(const Ray& ray, double& t, int& id){
	t = 1e20;
	id = -1;

	for(int i =0; i < spheres.size(); i++){
		const Sphere& s = spheres[i];
		Vec3 oc = ray.origin - s.center;

		double a = ray.dir.dot(ray.dir);
		double b = 2.0* oc.dot(ray.dir);
	    double c = oc.dot(oc) - s.radius*s.radius; 	

		double disc = b*b - 4.0*a*c;

		if(disc > 0) {
			double sqrt_disc = std::sqrt(disc);
			double val_1 = (-b + sqrt_disc)/(2.0*a);
			double val_2 = (-b - sqrt_disc)/(2.0*a);
			if(val_1 > 0.001 && val_1 < t) {t = val_1; id = i;}
			if(val_2 > 0.001 && val_2 < t) {t = val_2; id = i;}
	
		}
		
	}

	return id != -1;
}

Vec3 trace(const Ray& ray, int depth = 0){
	if(depth > 50) return Vec3(0, 0, 0);

	double t;
	int id;

	if(!intersect(ray, t, id)){
		return Vec3(0.2, 0.4, 0.8);
	}

	const Sphere& sp =	spheres[id];
	Vec3 hit_point = ray.origin + ray.dir*t;
	Vec3 normal = (hit_point - sp.center).normalize();

	Vec3 L = sp.emission;

	double p = std::max({sp.color.x, sp.color.y, sp.color.z});
	if(depth > 4){
		if(random_double() > p) return L;	
	}else{
		p = 1.0;
	}

	Vec3 next_dir = random_cosine_direction(normal);
	if(next_dir.dot(normal) < 0) next_dir = next_dir*-1.0;

	Ray next_ray(hit_point, next_dir);

	Vec3 throughput = sp.color*(1.0/ p);

	return L + throughput * trace(next_ray, depth + 1);
	
}

int main(){
	int width = 1280;
	int height = 720;
	int samples = 128;

	std::vector<Vec3> image(width*height);
	for(int j = 0; j < height; j++){
		for(int i = 0; i < width; i++)
		{
			Vec3 col = {0, 0, 0};              //accumulated color
			
			for(int s = 0; s < samples; s++){
				
				double u = (i + random_double())/width;
				double v = (j + random_double())/height;
				double aspect = (double)width / height;

				double px = (2.0 * u - 1.0) * aspect;
				double py = (1.0 - 2.0 * v);  

				Vec3 cam_dir = Vec3(px, py, -0.3).normalize();
				Ray new_ray(Vec3(0, 0, 0), cam_dir);

				col+= trace(new_ray);
			}
			
			image[j*width + i] = col*(1.0/samples);
		}
		if(j % 60 == 0) std::cout << "Current Progress" << (j*100/height) << "%\n";
	}
	std::ofstream file("output.ppm");
	file << "P3\n" << width << " " << height << "\n255\n";

	auto gamma_correct = [](double x){
    return std::pow(x, 1.0/2.2);
	};

	for(auto &c : image){
		int r = std::max(0, std::min(255, (int)(gamma_correct(c.x) * 255)));
		int g = std::max(0, std::min(255, (int)(gamma_correct(c.y) * 255)));
		int b = std::max(0, std::min(255, (int)(gamma_correct(c.z) * 255)));
		file << r << " " << g << " " << b << "\n";
	}
	file.close();

	std::cout << "image generated yayy" << std::endl;

	return 0;
}
