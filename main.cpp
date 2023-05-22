#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/core/utils/logger.hpp>
#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <set>
#include <list>
#include <map>
#include <typeinfo>
#include <fstream>

using cv::Mat;
using cv::samples::findFile;
using cv::IMREAD_COLOR;
using cv::waitKey;

using std::string;
using std::cout;
using std::cin;
using std::istream;
using std::ostream;
using std::endl;

// block of code to disable opencv warnings
int dummy_error_handler(int status, char const *func_name, char const *err_msg, char const *file_name, int line,
                        void *userdata) {
    //Do nothing -- will suppress console output
    return 0;   //Return value is not used
}

// sets warning display off
void set_dummy_error_handler() {
    cv::redirectError(dummy_error_handler);
}

// sets warning display on
void reset_error_handler() {
    cv::redirectError(nullptr);
}

void initOpenCV() {
    // for stopping logging info in console
    cv::utils::logging::setLogLevel(cv::utils::logging::LogLevel::LOG_LEVEL_SILENT);
    // for stopping warnings in console
    set_dummy_error_handler();
}

class Interface {
public:
    virtual void applyAll() = 0;
    virtual void write() const = 0;
    virtual istream &read(istream &in) = 0;
    virtual ostream &print(ostream &out) const = 0;
    virtual void serialize(string) const = 0;
    virtual void deserialize(std::ifstream&) = 0;
};

class Image : public Interface {
protected:
    // boolean for image finding
    bool absolute;
    string name, path;
    Mat img;
public:
    Image(string name = "cat.png", string path = "../Images/", bool absolute = false);
    Image(const Image &obj);
    Image &operator=(const Image &obj);
    // virtual to call derivative destructors
    virtual ~Image();

    istream &read(istream &in);
    ostream &print(ostream &out) const;
    friend istream &operator>>(istream &in, Image &obj);
    friend ostream &operator<<(ostream &out, const Image &obj);

    string extension(string word) const;
    string withoutExtension(string word) const;
    void scan();
    void show() const;
    void show(const Mat &img) const;
    void write() const;
    void saveShow() const;
    void applyAll();
    string getName() const;
    string getPath() const;

    bool operator<(const Image& obj) const {
        return !(this->name > obj.name);
    }
    bool operator==(const Image& obj) const {
        return this->name == obj.name;
    }
    virtual void serialize(string) const;
    virtual void deserialize(std::ifstream&);
};

void Image::serialize(string fileName) const {
    std::ofstream out(fileName, std::ios_base::app);

    out<<name<<" "<<path<<" "<<absolute<<" ";

    out.close();
}

void Image::deserialize(std::ifstream& in) {
    string name,path;
    bool absolute;
    in>>name>>path>>absolute;
    this->name = name;
    this->path = path;
    this->absolute = absolute;

    this->scan();
}

Image::Image(string name, string path, bool absolute) {
    this->name = name;
    this->path = path;
    this->absolute = absolute;
}

Image::Image(const Image &obj) {
    this->name = obj.name;
    this->path = obj.path;
    this->absolute = obj.absolute;
}

Image &Image::operator=(const Image &obj) {
    if (this != &obj) {
        if (!this->name.empty()) this->name.clear();
        this->name = obj.name;
        if (!this->path.empty()) this->path.clear();
        this->path = obj.path;
        this->absolute = obj.absolute;
    }
    return *this;
}

istream &Image::read(istream &in) {
    if (!this->name.empty()) this->name.clear();
    if (!this->path.empty()) this->path.clear();
    cout << "Enter name: \n";
    in >> this->name;
    cout << "Do you want to use relative path? (1:yes 0:no)?\n";
    int temp;
    cin >> temp;
    in.get();
    if (temp == 0) {
        this->absolute = true;
        cout << "Enter path to image: \n";
        // to get \n from buffer

        getline(in, this->path);
        // deleting "" from path
        if (this->path[0] == '"') this->path.erase(0, 1), this->path.pop_back();
    } else this->path = "../Images/";

    return in;
}

ostream &Image::print(ostream &out) const {
    out << "Name: " << this->name << endl;
    if (this->absolute == false) out << "Path to image: " << this->path + this->name << endl;
    else out << "Path to image: " << this->path << endl;

    return out;
}

istream &operator>>(istream &in, Image &obj) {
    return obj.read(in);
}

ostream &operator<<(ostream &out, const Image &obj) {
    return obj.print(out);
}

Image::~Image() {
    if (!this->name.empty()) this->name.clear();
    if (!this->path.empty()) this->path.clear();
    this->absolute = false;
}

string Image::extension(string word) const {
    return word.substr(word.find('.'), word.length());
}

string Image::withoutExtension(string word) const {
    return word.substr(0, word.find('.'));
}

void Image::scan() {
    string image_path;
    try {
        if (this->absolute == false) {
            string full_name = this->path + this->name;
            // silent mode true to suppress errors
            image_path = findFile(full_name, true, true);
        } else image_path = findFile(this->path, true, true);

        Mat temp = imread(image_path, IMREAD_COLOR);
        img.create(temp.rows, temp.cols, temp.type());
        cv::resize(img, img, temp.size());
        temp.copyTo(img);
    }
    catch (...) { cout << "~ INVALID PATH\n"; }
    // CV_8UC3 = 8 bit unsigned integer with 3 channels (RGB)
}

void Image::show() const {
    try {
//        Mat img = this->scan();
        cv::namedWindow("Image", cv::WINDOW_NORMAL);
//        using this function makes the window not have a title bar
//        cv::setWindowProperty("Image",cv::WND_PROP_FULLSCREEN, cv::WINDOW_FULLSCREEN);
        double aspect_ratio = static_cast<double>(img.cols) / img.rows;
        cv::resizeWindow("Image", static_cast<int>(540 * aspect_ratio), 540);
        imshow("Image", img);

//        Wait for a keystroke in the window
        int k = waitKey(0);
//        27 is ascii code for esc and waitKey returns an int
        if (k == 27) {
            cv::destroyAllWindows();
            return;
        }
    }
    catch (...) { cout << "~ OUTPUT FAILED\n"; }
}

void Image::show(const Mat &img) const {
    try {
        cv::namedWindow("Image", cv::WINDOW_NORMAL);
//        using this function makes the window not have a title bar
//        cv::setWindowProperty("Image",cv::WND_PROP_FULLSCREEN, cv::WINDOW_FULLSCREEN);
        double aspect_ratio = static_cast<double>(img.cols) / img.rows;
        cv::resizeWindow("Image", static_cast<int>(540 * aspect_ratio), 540);
        imshow("Image", img);

//        Wait for a keystroke in the window
        int k = waitKey(0);
//        27 is ascii code for esc and waitKey returns an int
        if (k == 27) {
            cv::destroyAllWindows();
            return;
        }
    }
    catch (...) { cout << "~ OUTPUT FAILED\n"; }
}

void Image::write() const {
    try {
        // basically does nothing because there is nothing applied to that image
//        Mat img = this->scan();
        string full_path = this->path + this->name;
        cv::imwrite(full_path, img);
    }
    catch (...) { cout << "~ WRITING IMAGE FAILED\n"; }
}

void Image::saveShow() const {
    cout << "Show image on screen (yes:1 no:0)?\n";
    int temp;
    cin >> temp;
    cin.get();
    if (temp == 1) this->show();

    cout << "Save image (yes:1 no:0)?\n";
    cin >> temp;
    cin.get();
    if (temp == 1) this->write();
}

void Image::applyAll() {
    cout << "~ NOTHING TO APPLY\n";
}

string Image::getName() const {
    return name;
}

string Image::getPath() const {
    return path;
}

class Effect : virtual public Image {
protected:
    int blurAmount;
    bool effect, blackWhite, cartoon;
public:
    Effect(string name = "cat.png", string path = "../Images/", bool absolute = false, bool effect = false,
           int blurAmount = 0, bool blackWhite = false, bool cartoon = false);
    Effect(const Effect &obj);
    Effect &operator=(const Effect &obj);

    // override specifier ensures that the function is virtual and is overriding a virtual function from a base class
    virtual ~Effect() override;
    istream &read(istream &in);
    ostream &print(ostream &out) const;

    void blur();
    void bw();
    void cartoon_effect();
    void write() const;
    void applyAll();
    void setBlurAmount(int blurAmount);
    void setBlackWhite(bool blackWhite);
    void setCartoon(bool cartoon);

    void serialize(string) const;
    void deserialize(std::ifstream&);
};

void Effect::serialize(string fileName) const {
    this->Image::serialize(fileName);
    std::ofstream out(fileName, std::ios_base::app);

    out<<effect<<" "<<blurAmount<<" "<<blackWhite<<" "<<cartoon<<" ";

    out.close();
}

void Effect::deserialize(std::ifstream& in) {
    this->Image::deserialize(in);

    bool effect,blackWhite,cartoon;
    int blurAmount;

    in>>effect>>blurAmount>>blackWhite>>cartoon;
    this->effect = effect;
    this->blurAmount = blurAmount;
    this->blackWhite = blackWhite;
    this->cartoon = cartoon;
}

Effect::Effect(string name, string path, bool absolute, bool effect, int blurAmount, bool blackWhite, bool cartoon) :
        Image(name, path, absolute) {
    this->effect = effect;
    this->blurAmount = blurAmount;
    this->blackWhite = blackWhite;
    this->cartoon = cartoon;
    this->scan();
}

Effect::Effect(const Effect &obj) : Image(obj) {
    this->effect = obj.effect;
    this->blurAmount = obj.blurAmount;
    this->blackWhite = obj.blackWhite;
    this->cartoon = obj.cartoon;
    this->scan();
}

Effect &Effect::operator=(const Effect &obj) {
    if (this != &obj) {
        Image::operator=(obj);
        this->effect = obj.effect;
        this->blurAmount = obj.blurAmount;
        this->blackWhite = obj.blackWhite;
        this->cartoon = obj.cartoon;
        this->scan();
    }
    return *this;
}

istream &Effect::read(istream &in) {
    this->Image::read(in);
    cout << "Are there effects applied on the image? (yes:1 no:0) \n";
    in >> this->effect;
    cout << "Do you want to blur the image? (yes:1 no:0)?\n";
    int temp;
    in >> temp;
    in.get();
    if (temp == 1) {
        cout << "Enter blur amount: \n";
        in >> this->blurAmount;
    }
    cout << "Do you want to apply Black and White effect to the image? (yes:1 no:0)?\n";
    in >> this->blackWhite;
    cout << "Do you want to apply Cartoon effect to the image? (yes:1 no:0)?\n";
    in >> this->cartoon;
    this->scan();

    return in;
}

ostream &Effect::print(ostream &out) const {
    this->Image::print(out);
    if (this->effect == true) out << "Has effects applied\n";
    else out << "Doesn't have effects applied\n";
    out << "Blur amount: " << this->blurAmount << endl;
    if (this->blackWhite == true) out << "Has Black and White effect applied\n";
    else out << "Doesn't have Black and White effect applied\n";
    if (this->cartoon) out << "Has Cartoon effect applied\n";
    else out << "Doesn't have Cartoon effect applied\n";

    return out;
}

Effect::~Effect() {
    this->effect = false;
    this->blurAmount = 0;
}

void Effect::write() const {
    try {
        string full_path = "../Images with Effects/" + this->withoutExtension(this->name) + "_withEffects" +
                           this->extension(this->name);
        cv::imwrite(full_path, this->img);
    }
    catch (...) { cout << "~ WRITING IMAGE FAILED\n"; }
}

void Effect::blur() {
    if (this->blurAmount > 0) {
        try {
            Mat blurredImage;
            // cv::GaussianBlur doesnt work with widths and heigths that are even, or 0,0
            if (this->blurAmount % 2 == 0) this->blurAmount += 1;

            cv::GaussianBlur(this->img, this->img, cv::Size(this->blurAmount, this->blurAmount), 0);
//        blurredImage.copyTo(this->img);
            this->effect = true;
        }
        catch (...) { cout << "~ APPLYING EFFECT FAILED\n"; }
    }
}

void Effect::bw() {
    if (this->blackWhite == true) {
        try {
            cv::cvtColor(img, img, cv::COLOR_BGR2GRAY);
            this->effect = true;
        }
        catch (...) { cout << "~ APPLYING EFFECT FAILED\n"; }
    }
}

void Effect::cartoon_effect() {
    if (this->cartoon == true) {
        try {
            Mat gray, tresh, edges;
            // to check if the image is already gray
            if (img.channels() != 1) cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);
            else img.copyTo(gray);

            // blur image to get a better mask for outlines
            cv::medianBlur(gray, gray, 7);
            // create outline using a treshold
            cv::adaptiveThreshold(gray, tresh, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 21, 7);

            // blur initial image with a safer method
            cv::bilateralFilter(img, edges, 21, 250, 250);

            // combine initial blurred image with the outlines
            cv::bitwise_and(edges, edges, img, tresh);
            this->effect = true;
        }
        catch (...) { cout << "~ APPLYING EFFECT FAILED\n"; }
    }
}

void Effect::applyAll() {
    this->blur();
    this->bw();
    this->cartoon_effect();
}

void Effect::setBlurAmount(int blurAmount) {
    this->blurAmount = blurAmount;
}

void Effect::setBlackWhite(bool blackWhite) {
    this->blackWhite = blackWhite;
}

void Effect::setCartoon(bool cartoon) {
    this->cartoon = cartoon;
}

class Adjustment : virtual public Image {
protected:
    double brightness, contrast;
    int hue;
    bool adjustment;
public:
    Adjustment(string name = "cat.png", string path = "../Images/", bool absolute = false, bool adjustment = false,
               double brightness = 0, double contrast = 1, int hue = 0);
    Adjustment(const Adjustment &obj);
    Adjustment &operator=(const Adjustment &obj);

    // override specifier ensures that the function is virtual and is overriding a virtual function from a base class
    virtual ~Adjustment() override;
    istream &read(istream &in);
    ostream &print(ostream &out) const;

    void brightness_adjustment();
    void contrast_adjustment();
    void hue_adjustment();
    void write() const;
    void applyAll();
    void setBrightness(double brightness);
    void setContrast(double contrast);
    void setHue(int hue);

    void serialize(string) const;
    void deserialize(std::ifstream&);
};

void Adjustment::serialize(string fileName) const {
    this->Image::serialize(fileName);
    std::ofstream out(fileName, std::ios_base::app);

    out<<adjustment<<" "<<brightness<<" "<<contrast<<" "<<hue<<" ";

    out.close();
}

void Adjustment::deserialize(std::ifstream& in) {
    this->Image::deserialize(in);

    bool adjustment;
    double brightness, contrast;
    int hue;

    in>>adjustment>>brightness>>contrast>>hue;
    this->adjustment = adjustment;
    this->brightness = brightness;
    this->contrast = contrast;
    this->hue = hue;
}

Adjustment::Adjustment(string name, string path, bool absolute, bool adjustment,
                       double brightness, double contrast, int hue) :
        Image(name, path, absolute) {
    this->adjustment = adjustment;
    this->brightness = brightness;
    this->contrast = contrast;
    this->hue = hue;
    this->scan();
}

Adjustment::Adjustment(const Adjustment &obj) : Image(obj) {
    this->adjustment = obj.adjustment;
    this->brightness = obj.brightness;
    this->contrast = obj.contrast;
    this->hue = obj.hue;
    this->scan();
}

Adjustment &Adjustment::operator=(const Adjustment &obj) {
    if (this != &obj) {
        Image::operator=(obj);
        this->adjustment = obj.adjustment;
        this->brightness = obj.brightness;
        this->contrast = obj.contrast;
        this->hue = obj.hue;
        this->scan();
    }
    return *this;
}

istream &Adjustment::read(istream &in) {
    this->Image::read(in);
    cout << "Is the image adjusted? (yes:1 no:0)\n";
    in >> this->adjustment;
    in.get();
    cout << "Enter brightness [-100,100]: \n";
    in >> this->brightness;
    in.get();
    cout << "Enter contrast [0,10]: \n\t1 = nothing changes\n\t[0,1) = lower contrast\n\t(1,10] = higher contrast\n";
    in >> this->contrast;
    in.get();
    cout << "Enter hue [0,180]: \n";
    in >> this->hue;
    in.get();
    this->scan();

    return in;
}

ostream &Adjustment::print(ostream &out) const {
    this->Image::print(out);
    if (this->adjustment == true) out << "Has adjustments applied\n";
    else out << "Doesn't have adjustments applied\n";
    out << "Brightness value: " << this->brightness << endl;
    out << "Contrast value: " << this->contrast << endl;
    out << "Hue value: " << this->hue << endl;

    return out;
}

Adjustment::~Adjustment() {
    this->contrast = 0;
    this->brightness = 0;
    this->hue = 0;
    this->adjustment = false;
}

void Adjustment::write() const {
    try {
        string full_path = "../Images with Adjustments/" + this->withoutExtension(this->name) + "_withAdjustments" +
                           this->extension(this->name);
        cv::imwrite(full_path, this->img);
    }
    catch (...) { cout << "~ WRITING IMAGE FAILED\n"; }
}

void Adjustment::brightness_adjustment() {
    if (this->brightness != 0 && this->brightness >= -100 && this->brightness <= 100) {
        try {
            // rtype == -1 means same type as source image
            // alpha = contrast, beta = brightness
            img.convertTo(img, -1, 1, this->brightness);
            this->adjustment = true;
        }
        catch (...) { cout << "~ APPLYING ADJUSTMENT FAILED\n"; }
    }
}

void Adjustment::contrast_adjustment() {
    if (this->contrast >= 0 && this->contrast <= 10) {
        try {
            // rtype == -1 means same type as source image
            // alpha = contrast, beta = brightness
            img.convertTo(img, -1, this->contrast, 0);
            this->adjustment = true;
        }
        catch (...) { cout << "~ APPLYING ADJUSTMENT FAILED\n"; }
    }
}

void Adjustment::hue_adjustment() {
    if (this->hue != 0 && this->hue >= 0 && this->hue <= 180) {
        try {
            Mat hsv_img;
            // changing color space to HSV (HUE, SATURATION, VALUE)
            cv::cvtColor(img, hsv_img, cv::COLOR_BGR2HSV);

            for (int i = 0; i < hsv_img.rows; i++)
                for (int j = 0; j < hsv_img.cols; j++) {
                    // to extract hue of the current pixel
                    int h = hsv_img.at<cv::Vec3b>(i, j)[0];
                    // adding value of this->hue to h
                    h = (h + this->hue) % 180;
                    // changing pixel hue
                    hsv_img.at<cv::Vec3b>(i, j)[0] = h;
                }
            // converting back to original color space
            cv::cvtColor(hsv_img, img, cv::COLOR_HSV2BGR);
            this->adjustment = true;
        }
        catch (...) { cout << "~ APPLYING ADJUSTMENT FAILED\n"; }
    }
}

void Adjustment::applyAll() {
    this->brightness_adjustment();
    this->contrast_adjustment();
    this->hue_adjustment();
}

void Adjustment::setBrightness(double brightness) {
    this->brightness = brightness;
}

void Adjustment::setContrast(double contrast) {
    this->contrast = contrast;
}

void Adjustment::setHue(int hue) {
    this->hue = hue;
}

class Edited : public Effect, public Adjustment {
private:
    bool edited;
    string date;
public:
    Edited(string name = "cat.png", string path = "../Images/", bool absolute = false, bool effect = false,
           int blurAmount = 0, bool blackWhite = false, bool cartoon = false,
           bool adjustment = false, double brightness = 0, double contrast = 1, int hue = 0, bool edited = false,
           string date = "13/06/1826");
    Edited(const Edited &obj);
    Edited &operator=(const Edited &obj);

    ~Edited();
    istream &read(istream &in);
    ostream &print(ostream &out) const;

    void write() const;
    void applyAll();
    void serialize(string) const;
    void deserialize(std::ifstream&);
};

void Edited::serialize(string fileName) const {
    this->Effect::serialize(fileName);
    std::ofstream out(fileName, std::ios_base::app);

    out<<adjustment<<" "<<brightness<<" "<<contrast<<" "<<hue<<" "<<edited<<" "<<date;

    out.close();
}

void Edited::deserialize(std::ifstream& in) {
    this->Effect::deserialize(in);

    bool adjustment,edited;
    double brightness, contrast;
    int hue;
    string date;

    in>>adjustment>>brightness>>contrast>>hue>>edited>>date;
    this->adjustment = adjustment;
    this->brightness = brightness;
    this->contrast = contrast;
    this->hue = hue;
    this->edited = edited;
    this->date = date;
}

Edited::Edited(string name, string path, bool absolute, bool effect, int blurAmount, bool blackWhite, bool cartoon,
               bool adjustment, double brightness, double contrast, int hue, bool edited, string date) : Image(name,path,absolute),
                                                                                                         Effect(name,path,absolute,effect,blurAmount,blackWhite,cartoon),
                                                                                                         Adjustment(name,path,absolute,adjustment,brightness,contrast,hue) {
    this->edited = edited;
    this->date = date;
    this->scan();
}

Edited::Edited(const Edited &obj) : Image(obj), Effect(obj), Adjustment(obj) {
    this->edited = obj.edited;
    this->date = obj.date;
    this->scan();
}

Edited &Edited::operator=(const Edited &obj) {
    if (this != &obj) {
        Effect::operator=(obj);
        Adjustment::operator=(obj);
        this->edited = obj.edited;
        this->date = obj.date;
        this->scan();
    }
    return *this;
}

Edited::~Edited() {
    this->edited = false;
    if (!this->date.empty()) this->date.clear();
}

istream &Edited::read(istream &in) {
    this->Effect::read(in);

    cout << "Is the image adjusted? (yes:1 no:0)\n";
    in >> this->adjustment;
    in.get();
    cout << "Enter brightness [-100,100]: \n";
    in >> this->brightness;
    in.get();
    cout << "Enter contrast [0,10]: \n\t1 = nothing changes\n\t[0,1) = lower contrast\n\t(1,10] = higher contrast\n";
    in >> this->contrast;
    in.get();
    cout << "Enter hue [0,180]: \n";
    in >> this->hue;
    in.get();

    cout << "Is the image edited (yes:1 no:0)?\n";
    in >> this->edited;
    in.get();
    cout << "Enter date of edited image: \n";
    std::getline(in, this->date);

    this->scan();
    return in;
}

ostream &Edited::print(ostream &out) const {
    this->Effect::print(out);

    if (this->adjustment == true) out << "Has adjustments applied\n";
    else out << "Doesn't have adjustments applied\n";
    out << "Brightness value: " << this->brightness << endl;
    out << "Contrast value: " << this->contrast << endl;
    out << "Hue value: " << this->hue << endl;

    if (this->edited == true) out << "Is edited\n";
    else out << "Is not edited\n";

    out << "Date of completion: " << this->date << endl;

    return out;
}

void Edited::write() const {
    try {
        string full_path =
                "../Edited Images/" + this->withoutExtension(this->name) + "_Edited" + this->extension(this->name);
        cv::imwrite(full_path, this->img);
    }
    catch (...) { cout << "~ WRITING IMAGE FAILED\n"; }
}

void Edited::applyAll() {
    this->brightness_adjustment();
    this->contrast_adjustment();
    this->hue_adjustment();

    this->blur();
    this->bw();
    this->cartoon_effect();
}

class Photoshop {
private:
    Image *image;
    bool favorite, goBack;

public:
    Image *getImage() { return this->image; }
    Image*& getImageByReference() {return this->image;}
    friend istream &operator>>(istream &in, Photoshop &obj);
    friend ostream &operator<<(ostream &out, const Photoshop &obj);

    bool isGoBack() const;

    // methods for template
    void scan(){image->scan();}
    void write() const {image->write();}
    void show() const {image->show();}
    void applyAll(){image->applyAll();}

    // setters for template
    void setBlurAmount(int);
    void setBlackWhite(bool);
    void setCartoon(bool);
    void setBrightness(double);
    void setContrast(double);
    void setHue(int);

    string getType() {
        return typeid(*image).name();
    }
    void serialize(string) const;
    void deserialize(std::ifstream&);

    bool operator<(const Photoshop& obj) const {
        return *(this->image) < *(obj.image);
    }
    bool operator==(const Photoshop& obj) const {
        return *(this->image) == *(obj.image);
    }
};

void Photoshop::serialize(string fileName) const {
    image->serialize(fileName);
}

void Photoshop::deserialize(std::ifstream& in) {
    image->deserialize(in);
}

void Photoshop::setBrightness(double brightness) {
    if (typeid(*image) == typeid(Adjustment) || typeid(*image) == typeid(Edited)) {
        dynamic_cast<Adjustment&>(*image).setBrightness(brightness);
        std::cout << "~ ADJUSTMENT WAS APPLIED SUCCESSFULLY\n";
    } else std::cout << "~ OBJECT IS NOT OF TYPE ADJUSTMENT OR EDITING\n";
}

void Photoshop::setContrast(double contrast) {
    if (typeid(*image) == typeid(Adjustment) || typeid(*image) == typeid(Edited)) {
        dynamic_cast<Adjustment&>(*image).setContrast(contrast);
        std::cout << "~ ADJUSTMENT WAS APPLIED SUCCESSFULLY\n";
    } else std::cout << "~ OBJECT IS NOT OF TYPE ADJUSTMENT OR EDITING\n";
}

void Photoshop::setHue(int hue) {
    if (typeid(*image) == typeid(Adjustment) || typeid(*image) == typeid(Edited)) {
        dynamic_cast<Adjustment&>(*image).setHue(hue);
        std::cout << "~ ADJUSTMENT WAS APPLIED SUCCESSFULLY\n";
    } else std::cout << "~ OBJECT IS NOT OF TYPE ADJUSTMENT OR EDITING\n";
}

void Photoshop::setBlurAmount(int blurAmount) {
    if (typeid(*image) == typeid(Effect) || typeid(*image) == typeid(Edited)) {
        dynamic_cast<Effect&>(*image).setBlurAmount(blurAmount);
        std::cout << "~ EFFECT WAS APPLIED SUCCESSFULLY\n";
    } else std::cout << "~ OBJECT IS NOT OF TYPE EFFECT OR EDITING\n";
}

void Photoshop::setBlackWhite(bool blackWhite) {
    if (typeid(*image) == typeid(Effect) || typeid(*image) == typeid(Edited)) {
        dynamic_cast<Effect&>(*image).setBlackWhite(blackWhite);
        std::cout<< "~ EFFECT WAS APPLIED SUCCESSFULLY\n";
    }
    else std::cout << "~ OBJECT IS NOT OF TYPE EFFECT OR EDITING\n";
}

void Photoshop::setCartoon(bool cartoon) {
    if (typeid(*image) == typeid(Effect) || typeid(*image) == typeid(Edited)) {
        dynamic_cast<Effect&>(*image).setCartoon(cartoon);
        std::cout<< "~ EFFECT WAS APPLIED SUCCESSFULLY\n";
    }
    else std::cout << "~ OBJECT IS NOT OF TYPE EFFECT OR EDITING\n";
}

istream &operator>>(istream &in, Photoshop &obj) {
    obj.goBack = false;

    for (int i = 0; i < 10; i++) cout << "-";
    cout << " CREATE ";
    for (int i = 0; i < 10; i++) cout << "-";
    cout << endl;

    cout << "1. Effects\n";
    cout << "2. Adjustments\n";
    cout << "3. Editing\n";
    cout << "0. Go back\n";

    int temp;
    cin >> temp;
    cin.get();

    switch (temp) {
        case 0: {
            obj.goBack = true;
            break;
        }
        case 1: {
            obj.image = new Effect();
            break;
        }
        case 2: {
            obj.image = new Adjustment();
            break;
        }
        case 3: {
            obj.image = new Edited();
            break;
        }
        default:
            cout << "~ INVALID OPTION\n";
    }

    if (obj.goBack == false) {
        in >> *obj.image;
        cout << "Is this a favorite image (yes:1 no:0)?\n";
        in >> obj.favorite;
    }

    return in;
}

ostream &operator<<(ostream &out, const Photoshop &obj) {
    out << *obj.image;

    if (obj.favorite == true) out << "Is a favorite image\n";
    else out << "Is not a favorite image\n";

    return out;
}

bool Photoshop::isGoBack() const {
    return this->goBack;
}

class Video {
private:
    static int counter;
    const int id;
    string name;
    double fps;
    int blurAmount, hue;
    bool blackWhite, cartoon;
    double brightness, contrast;
    cv::VideoCapture capture;
    std::vector<Mat> sequence;
public:
    Video(const string &name = "", double fps = 0.0, int blurAmount = 0, bool blackWhite = false,
          bool cartoon = false, double brightness = 0, double contrast = 1, int hue = 0);
    Video(const Video &obj);
    ~Video();
    Video &operator=(const Video &obj);
    friend istream &operator>>(istream &in, Video &obj);
    friend ostream &operator<<(ostream &out, const Video &obj);

    bool operator<(const Video& obj) const {
        return this->id <= obj.id;
    }
    bool operator==(const Video& obj) const {
        return (this->id == obj.id);
    }

//    methods
    void scan();
    void write() const;
    void show() const;
    void applyAll();
    void blur();
    void bw();
    void cartoon_effect();
    void brightness_adjustment();
    void contrast_adjustment();
    void hue_adjustment();

//    setters
    void setBlurAmount(int blurAmount);
    void setBlackWhite(bool blackWhite);
    void setCartoon(bool cartoon);
    void setBrightness(double brightness);
    void setContrast(double contrast);
    void setHue(int hue);

    string getType(){return typeid(*this).name();}
    void serialize(string) const;
    void deserialize(std::ifstream&);
};

void Video::serialize(string fileName) const {
    std::ofstream out(fileName, std::ios_base::app);

    out<<name<<" "<<blurAmount<<" "<<blackWhite<<" "<<cartoon<<" "<<brightness<<" "<<contrast<<" "<<hue;

    out.close();
}

void Video::deserialize(std::ifstream& in) {
    string name;
    bool blackWhite,cartoon;
    int blurAmount,hue;
    double brightness, contrast;

    in>>name>>blurAmount>>blackWhite>>cartoon>>brightness>>contrast>>hue;
    this->name = name;
    this->blurAmount = blurAmount;
    this->blackWhite = blackWhite;
    this->cartoon = cartoon;
    this->brightness = brightness;
    this->contrast = contrast;
    this->hue = hue;
}

int Video::counter = 0;

Video::Video(const string &name, double fps, int blurAmount, bool blackWhite,
             bool cartoon, double brightness, double contrast, int hue) : id(counter++) {
    if (name.empty()) {
        this->name = "video" + std::to_string(id);
        this->name += ".mp4";
    }
    else this->name = name;
    this->fps = fps;
    this->blurAmount = blurAmount;
    this->hue = hue;
    this->blackWhite = blackWhite;
    this->cartoon = cartoon;
    this->brightness = brightness;
    this->contrast = contrast;
//    to open the laptop camera
    this->capture.open(0);
}

Video::Video(const Video &obj) : id(counter++) {
    if (obj.name.empty()) {
        this->name = "video" + std::to_string(id);
        this->name += ".mp4";
    }
    else this->name = obj.name;
    this->fps = obj.fps;
    this->blurAmount = obj.blurAmount;
    this->hue = obj.hue;
    this->blackWhite = obj.blackWhite;
    this->cartoon = obj.cartoon;
    this->brightness = obj.brightness;
    this->contrast = obj.contrast;
    this->capture = capture;
    this->sequence = sequence;
}

Video::~Video() {
    counter = 0;
    if (!name.empty()) name.clear();
    fps = 0.0;
    blurAmount = 0;
    hue = 0;
    blackWhite = false;
    cartoon = false;
    brightness = 0;
    contrast = 1;
    capture.release();
    if (!sequence.empty()) sequence.clear();
}

Video &Video::operator=(const Video &obj) {
    if (this != &obj) {
        if (obj.name.empty()) {
            this->name = "video" + std::to_string(id);
            this->name += ".mp4";
        }
        else this->name = obj.name;
        this->fps = obj.fps;
        this->blurAmount = obj.blurAmount;
        this->hue = obj.hue;
        this->blackWhite = obj.blackWhite;
        this->cartoon = obj.cartoon;
        this->brightness = obj.brightness;
        this->contrast = obj.contrast;
        this->capture = capture;
        this->sequence = sequence;
    }
    return *this;
}

istream &operator>>(istream &in, Video &obj) {
    if (!obj.name.empty()) obj.name.clear();
    cout << "Enter name: \n";
    in >> obj.name;

    cout << "Do you want to blur the video? (yes:1 no:0)?\n";
    int temp;
    in >> temp;
    in.get();
    if (temp == 1) {
        cout << "Enter blur amount: \n";
        in >> obj.blurAmount;
    }
    cout << "Do you want to apply Black and White effect? (yes:1 no:0)?\n";
    in >> obj.blackWhite;
    cout << "Do you want to apply Cartoon effect? (yes:1 no:0)?\n";
    in >> obj.cartoon;

    in.get();
    cout << "Enter brightness [-100,100]: \n";
    in >> obj.brightness;
    in.get();
    cout << "Enter contrast [0,10]: \n\t1 = nothing changes\n\t[0,1) = lower contrast\n\t(1,10] = higher contrast\n";
    in >> obj.contrast;
    in.get();
    cout << "Enter hue [0,180]: \n";
    in >> obj.hue;
    in.get();

    obj.scan();
    return in;
}

ostream &operator<<(ostream &out, const Video &obj) {
    out << obj.name << endl;

    out << "Blur amount: " << obj.blurAmount << endl;
    if (obj.blackWhite == true) out << "Has Black and White effect applied\n";
    else out << "Doesn't have Black and White effect applied\n";
    if (obj.cartoon) out << "Has Cartoon effect applied\n";
    else out << "Doesn't have Cartoon effect applied\n";

    out << "Brightness value: " << obj.brightness << endl;
    out << "Contrast value: " << obj.contrast << endl;
    out << "Hue value: " << obj.hue << endl;
    return out;
}

void Video::scan() {
    std::chrono::high_resolution_clock::time_point wasted_end;
    std::chrono::high_resolution_clock::time_point wasted_start;
    this->capture.open(0);
    if(!this->sequence.empty()) this->sequence.clear();
    // takes 35ms for camera to start which can be seen at low length videos
    // thats why i took in account the wasted time for it
    try {
        if (!capture.isOpened()) throw "~ Failed to open camera";
        cv::Mat frame;
        bool started = false;
        auto start = std::chrono::high_resolution_clock::now();
        wasted_start = std::chrono::high_resolution_clock::now();
        auto time_elapsed_start = std::chrono::high_resolution_clock::now();
        auto last_time_output = std::chrono::duration_cast<std::chrono::seconds>(time_elapsed_start - start).count();
        while (capture.read(frame)) {
            if (!started) {
                started = true;
                wasted_end = std::chrono::high_resolution_clock::now();
            }
            sequence.push_back(frame.clone());

            cv::imshow("Camera feed", frame);

            // to display video duration
            auto time_elapsed_end = std::chrono::high_resolution_clock::now();
            if (last_time_output !=
                std::chrono::duration_cast<std::chrono::seconds>(time_elapsed_end - time_elapsed_start).count()) {
                last_time_output = std::chrono::duration_cast<std::chrono::seconds>(
                        time_elapsed_end - time_elapsed_start).count();
                system("CLS");
                std::cout << "Video duration: " << std::chrono::duration_cast<std::chrono::seconds>(
                        time_elapsed_end - time_elapsed_start).count() << " seconds";
            }

            if (waitKey(1) == 27) break;
        }
        system("CLS");
        // recored time it took, and how many frames there are to get the fps of the video
        // because the camera won't share that info with opencv
        auto end = std::chrono::high_resolution_clock::now() - (wasted_end - wasted_start);
        this->fps = sequence.size() / (std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() / 1000.0);
        capture.release();
        cv::destroyAllWindows();
    }
    catch (string err) {
        std::cout << err << endl;
    }
}

void Video::write() const {
//    fourcc = video encode MJPG is for mp4 and avi
//    15 = fps (this is max for my webcam) , size for window, true because it has colors
    try {
        bool checkImage = true;
        if(sequence[0].channels() == 1) checkImage = false;
        cv::VideoWriter writer("../Videos/" + name, cv::VideoWriter::fourcc('m', 'p', '4', 'v'), fps,
                               cv::Size(sequence[0].cols, sequence[0].rows), checkImage);

//        with address so it won't copy each frame
        for (const auto &frame: sequence) writer.write(frame);

        if (!writer.isOpened()) throw string("~ Failed to open the video writer");

        writer.release();
    }
    catch (string err) {
        std::cout << err << endl;
    }
}

void Video::show() const {
    for (const auto &frame: sequence) {
        // showing each image individualy
        cv::imshow("Video", frame);
        // waiting found time before next frame is displayed
        // 1000.0 for division to work in double/float
        if (cv::waitKey(1000.0 / fps) == 27) break;
    }
    cv::destroyAllWindows();
}

void Video::blur() {
    if (blurAmount > 0)
        try {
            if (blurAmount % 2 == 0) blurAmount += 1;
            // parallelization of for loop to be faster
            // [&] = captures all variables used within lambda body and access them by reference
            std::cout << "~ LOADING [          ]";
            int fraction = floor(((double) sequence.size()) / 10);
            // atomic variable so one a thread cant read and another write in it at the same time
            std::atomic<int> counter = 0;
            // common variable across threads (like static but for threads)
            std::mutex printMutex;
            cv::parallel_for_(cv::Range(0, sequence.size()), [&](const cv::Range &range) {
                for (int i = range.start; i < range.end; i++) {
                    if (i % fraction == 0 && i != 0 && counter <= 10) {
                        // locks the variable until it goes out of scope
                        std::lock_guard<std::mutex> lock(printMutex);
                        counter++;
                        system("CLS");
                        std::cout << "~ LOADING [";
                        for (int j = 1; j <= counter; j++) std::cout << (char) 219;
                        for (int j = 10 - counter; j >= 1; j--) std::cout << " ";
                        std::cout << "]";
                    }

                    cv::GaussianBlur(sequence[i], sequence[i], cv::Size(blurAmount, blurAmount), 0);
                }
            });
            std::cout << "\n~ FINISHED\n";
        }
        catch (...) { cout << "~ APPLYING EFFECT FAILED\n"; }
}

void Video::bw() {
    if (blackWhite == true)
        try {
            std::cout << "~ LOADING [          ]";
            int fraction = floor(((double) sequence.size()) / 10);
            int counter = 0;
            // no parallelization here because it's a pretty fast effect
            for (int i = 0; i < sequence.size(); i++) {
                if (i % fraction == 0 && i != 0) {
                    counter++;
                    system("CLS");
                    std::cout << "~ LOADING [";
                    for (int j = 1; j <= counter; j++) std::cout << (char) 219;
                    for (int j = 10 - counter; j >= 1; j--) std::cout << " ";
                    std::cout << "]";
                }
                cv::cvtColor(sequence[i], sequence[i], cv::COLOR_BGR2GRAY);
            }
            std::cout << "\n~ FINISHED\n";
        }
        catch (...) { cout << "~ APPLYING EFFECT FAILED\n"; }
}

void Video::cartoon_effect() {
    if (cartoon == true)
        try {
            Mat gray, tresh, edges;
            std::cout << "~ LOADING [          ]";
            int fraction = floor(((double) sequence.size()) / 10);
            int counter = 0;
            for (int i = 0; i < sequence.size(); i++) {
                if (i % fraction == 0 && i != 0) {
                    counter++;
                    system("CLS");
                    std::cout << "~ LOADING [";
                    for (int j = 1; j <= counter; j++) std::cout << (char) 219;
                    for (int j = 10 - counter; j >= 1; j--) std::cout << " ";
                    std::cout << "]";
                }

                // to check if the image is already gray
                if (sequence[i].channels() != 1) cv::cvtColor(sequence[i], gray, cv::COLOR_BGR2GRAY);
                else sequence[i].copyTo(gray);

                // blur image to get a better mask for outlines
                cv::medianBlur(gray, gray, 7);
                // create outline using a treshold
                cv::adaptiveThreshold(gray, tresh, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 21, 7);
                // blur initial image with a safer method
                cv::bilateralFilter(sequence[i], edges, 21, 250, 250);
                // combine initial blurred image with the outlines
                cv::bitwise_and(edges, edges, sequence[i], tresh);
            }
            std::cout << "\n~ FINISHED\n";
        }
        catch (...) { cout << "~ APPLYING EFFECT FAILED\n"; }
}

void Video::brightness_adjustment() {
    if (brightness != 0) {
        try {
            if (brightness < -100 || brightness > 100) throw brightness;
            try {
                std::cout << "~ LOADING [          ]";
                int fraction = floor(((double) sequence.size()) / 10);
                int counter = 0;
                // no parallelization here because it's a pretty fast adjustment
                for (int i = 0; i < sequence.size(); i++) {
                    if (i % fraction == 0 && i != 0) {
                        counter++;
                        system("CLS");
                        std::cout << "~ LOADING [";
                        for (int j = 1; j <= counter; j++) std::cout << (char) 219;
                        for (int j = 10 - counter; j >= 1; j--) std::cout << " ";
                        std::cout << "]";
                    }
                    // rtype == -1 means same type as source image
                    // alpha = contrast, beta = brightness
                    sequence[i].convertTo(sequence[i], -1, 1, brightness);
                }
                std::cout << "\n~ FINISHED\n";
            }
            catch (...) { cout << "~ APPLYING ADJUSTMENT FAILED\n"; }
        }
        catch (double bright) {
            std::cout << "~ The brightness value: " << bright << " falls outside the valid range of [-100,100]\n";
        }
    }
}

void Video::contrast_adjustment() {
    if(contrast != 1)
        try {
            if (contrast < 0 && contrast > 10) throw contrast;
            try {
                std::cout << "~ LOADING [          ]";
                int fraction = floor(((double) sequence.size()) / 10);
                int temp;
                cin >> temp;
                int counter = 0;
                // no parallelization here because it's a pretty fast adjustment
                for (int i = 0; i < sequence.size(); i++) {
                    if (i % fraction == 0 && i != 0) {
                        counter++;
                        system("CLS");
                        std::cout << "~ LOADING [";
                        for (int j = 1; j <= counter; j++) std::cout << (char) 219;
                        for (int j = 10 - counter; j >= 1; j--) std::cout << " ";
                        std::cout << "]";
                    }
                    // rtype == -1 means same type as source image
                    // alpha = contrast, beta = brightness
                    sequence[i].convertTo(sequence[i], -1, contrast, 0);
                }
                std::cout << "\n~ FINISHED\n";
            }
            catch (...) { cout << "~ APPLYING ADJUSTMENT FAILED\n"; }
        }
        catch (double cont) {
            std::cout << "~ The contrast value: " << cont << " falls outside the valid range of [0,10]\n";
        }
}

void Video::hue_adjustment() {
    if (hue != 0)
        try {
            if (hue < 0 || hue > 180) throw hue;
            try {
                std::cout << "~ LOADING [          ]";
                int fraction = floor(((double) sequence.size()) / 10);
                int counter = 0;
                // no parallelization here because it's a pretty fast adjustment
                for (int i = 0; i < sequence.size(); i++) {
                    if (i % fraction == 0 && i != 0) {
                        counter++;
                        system("CLS");
                        std::cout << "~ LOADING [";
                        for (int j = 1; j <= counter; j++) std::cout << (char) 219;
                        for (int j = 10 - counter; j >= 1; j--) std::cout << " ";
                        std::cout << "]";
                    }
                    // rtype == -1 means same type as source image
                    // alpha = contrast, beta = brightness
                    Mat hsv;
                    // changing color space to HSV (HUE, SATURATION, VALUE)
                    cv::cvtColor(sequence[i], hsv, cv::COLOR_BGR2HSV);
                    for (int i = 0; i < hsv.rows; i++)
                        for (int j = 0; j < hsv.cols; j++) {
                            // to extract hue of the current pixel
                            int h = hsv.at<cv::Vec3b>(i, j)[0];
                            // adding value of this->hue to h
                            h = (h + this->hue) % 180;
                            // changing pixel hue
                            hsv.at<cv::Vec3b>(i, j)[0] = h;
                        }
                    // converting back to original color space
                    cv::cvtColor(hsv, sequence[i], cv::COLOR_HSV2BGR);
                }
                std::cout << "\n~ FINISHED\n";
            }
            catch (...) { cout << "~ APPLYING ADJUSTMENT FAILED\n"; }
        }
        catch (int h) { std::cout << "The hue value: " << h << " falls outside the valid range of [0,180]"; }
}

void Video::setBlurAmount(int blurAmount) {
    this->blurAmount = blurAmount;
}

void Video::setBlackWhite(bool blackWhite) {
    this->blackWhite = blackWhite;
}

void Video::setCartoon(bool cartoon) {
    this->cartoon = cartoon;
}

void Video::setBrightness(double brightness) {
    this->brightness = brightness;
}

void Video::setContrast(double contrast) {
    this->contrast = contrast;
}

void Video::setHue(int hue) {
    this->hue = hue;
}

void Video::applyAll() {
    this->contrast_adjustment();
    this->brightness_adjustment();
    this->hue_adjustment();

    this->blur();
    this->bw();
    this->cartoon_effect();
}

class MyException:public std::exception {
public:
    virtual const char* what() const throw() {
        return "~ FAILED TO IMPORT FILES\n";
    }
} importException;

template<class T>
class Project {
private:
    string name;
    T *current;
    std::list<T*> files; // to store data for fast deletes and insertions
    std::map<T*,int> versions; // contains version number of the file
public:
    Project() {
        name = "new project";
        versions[current] = 0;
        current = NULL;
    }
    ~Project() {
        if(!files.empty()) files.clear();
        if(!versions.empty()) versions.clear();
        if(versions.find(current) != versions.end()) versions.erase(current);
    }

    // doesnt work outside class
    friend std::istream& operator>>(std::istream &in, Project<T>& obj) {
        std::cout<<"Enter project name: \n";
        in>>obj.name;

        obj.menuEngine();

        return in;
    }
    friend std::ostream& operator<<(std::ostream &out, const Project<T>& obj) {
        std::cout<<"Project name: "<<obj.name<<"\n";

        return out;
    }

    bool operator<(const Project<T>& obj) const{
        return this->name < obj.name;
    }

    const string &getName() const {
        return name;
    }

    // template methods
    void displayOptions();
    void displayEngine();
    void displayMenu();
    void menuEngine();
    void displayEdit();
    void editEngine();
    void effectsEngine();
    void displayEffects();
    void adjustmentsEngine();
    void displayAdjusments();

    void write(string);
    void read(string);
};

template<class T>
void Project<T>::displayEffects() {
    std::cout<<"\tProject: "<<name<<"\n\tVersion:"<<versions[current]<<"\n";
    for (int i = 0; i < 10; i++) cout << "-";
    cout << " CHOOSE EFFECT ";
    for (int i = 0; i < 10; i++) cout << "-";
    cout << endl;

    cout << "1. Blur\n";
    cout << "2. Black and White\n";
    cout << "3. Cartoon\n";
    cout << "0. Go back\n";
}

template<class T>
void Project<T>::displayAdjusments() {
    std::cout<<"\tProject: "<<name<<"\n\tVersion:"<<versions[current]<<"\n";
    for (int i = 0; i < 10; i++) cout << "-";
    cout << " CHOOSE ADJUSTMENT ";
    for (int i = 0; i < 10; i++) cout << "-";
    cout << endl;

    cout << "1. Brightness\n";
    cout << "2. Contrast\n";
    cout << "3. Hue\n";
    cout << "0. Go back\n";;
}

template<class T>
void Project<T>::effectsEngine() {
    system("CLS");
    this->displayEffects();

    while (true) {
        int option;
        cout << "Enter option: \n";
        cin >> option;
        cin.get();
        switch (option) {
            case 1: {
                system("CLS");
                int temp;
                cout << "Enter blur amount: \n";
                cin >> temp;
                cin.get();
                current->setBlurAmount(temp);
                this->displayEffects();
                break;
            }
            case 2: {
                system("CLS");
                bool temp;
                cout << "Do you want to apply Black and White effect to the image (yes:1 no:0)?\n";
                cin >> temp;
                cin.get();
                current->setBlackWhite(temp);
                this->displayEffects();
                break;
            }
            case 3: {
                system("CLS");
                bool temp;
                cout << "Do you want to apply Cartoon effect to the image (yes:1 no:0)?\n";
                cin >> temp;
                cin.get();
                current->setCartoon(temp);
                this->displayEffects();
                break;
            }
            case 0: {
                system("CLS");
                return;
            }
            default:
                cout << "~ INVALID OPTION\n";
        }
    }
}

template<class T>
void Project<T>::adjustmentsEngine() {
    system("CLS");
    this->displayAdjusments();

    while (true) {
        int option;
        cout << "Enter option: \n";
        cin >> option;
        cin.get();
        switch (option) {
            case 1: {
                system("CLS");
                double temp;
                cout << "Enter Brightness [-100,100]: \n";
                cin >> temp;
                cin.get();
                current->setBrightness(temp);
                this->displayAdjusments();
                break;
            }
            case 2: {
                system("CLS");
                double temp;
                cout
                        << "Enter contrast [0,10]: \n\t1 = nothing changes\n\t[0,1) = lower contrast\n\t(1,10] = higher contrast\n";
                cin >> temp;
                cin.get();
                current->setContrast(temp);
                this->displayAdjusments();
                break;
            }
            case 3: {
                system("CLS");
                int temp;
                cout << "Enter hue [0,180]: \n";
                cin >> temp;
                cin.get();
                current->setHue(temp);
                this->displayAdjusments();
                break;
            }
            case 0: {
                return;
            }
            default:
                cout << "~ INVALID OPTION\n";
        }
    }
}

template<class T>
void Project<T>::displayEdit() {
    std::cout<<"\tProject: "<<name<<"\n\tVersion:"<<versions[current]<<"\n";
    for (int i = 0; i < 10; i++) cout << "-";
    cout << " CHOOSE OPTION ";
    for (int i = 0; i < 10; i++) cout << "-";
    cout << endl;

    cout << "1. Effects\n";
    cout << "2. Adjustments\n";
    cout << "3. Apply all changes\n";
    cout << "4. Reset\n";
    cout << "0. Go back\n";
}

template<class T>
void Project<T>::editEngine() {
    system("CLS");
    this->displayEdit();

    while (true) {
        int option;
        cout << "Enter option: \n";
        cin >> option;
        cin.get();
        switch (option) {
            case 1: {
                system("CLS");
                this->effectsEngine();
                this->displayEdit();
                break;
            }
            case 2: {
                system("CLS");
                this->adjustmentsEngine();
                this->displayEdit();
                break;
            }
            case 3: {
                system("CLS");
                current->applyAll();
                versions[current]++;
                cout << "~ CHANGES APPLIED SUCCESSFULLY\n";
                this->displayEdit();
                break;
            }
            case 4: {
                system("CLS");
                current->scan();
                versions[current] = 0;
                cout << "~ IMAGE RESET SUCCESSFULLY\n";
                this->displayEdit();
                break;
            }
            case 0: {
                system("CLS");
                return;
            }
            default:
                cout << "~ INVALID OPTION\n";
        }
    }
}

template<class T>
void Project<T>::displayOptions() {
    for (int i = 0; i < 10; i++) cout << "-";
    cout << " CHOOSE OPTION ";
    for (int i = 0; i < 10; i++) cout << "-";
    cout << endl;

    cout << "1. Info\n";
    cout << "2. Show\n";
    cout << "3. Save\n";
    cout << "0. Go back\n";
}

template<class T>
void Project<T>::displayEngine() {
    system("CLS");
    this->displayOptions();

    while (true) {
        int option;
        cout << "Enter option: \n";
        cin >> option;
        cin.get();
        switch (option) {
            case 1: {
                system("CLS");
                std::cout<<*current<<endl;
                this->displayOptions();
                break;
            }
            case 2: {
                system("CLS");
                current->show();
                this->displayOptions();
                break;
            }
            case 3: {
                system("CLS");
                current->write();
                cout << "~ FILE WAS SAVED SUCCESSFULLY\n";
                this->displayOptions();
                break;
            }
            case 0: {
                system("CLS");
                return;
            }
            default:
                cout << "~ INVALID OPTION\n";
        }
    }
}

template<class T>
void Project<T>::displayMenu() {
    std::cout<<"\tProject: "<<name<<"\n\tVersion:"<<versions[current]<<"\n";
    std::cout<<"1. Open\n";
    std::cout<<"2. Edit\n";
    std::cout<<"3. Delete\n";
    std::cout<<"4. Display\n";
    std::cout<<"0. Go Back\n";
}

template<class T>
void Project<T>::menuEngine() {
    system("CLS");
    this->displayMenu();

    while (true) {
        int option;
        cout << "Enter option: \n";
        cin >> option;
        cin.get();
        switch (option) {
            case 1: {
                system("CLS");
                std::cout<<"Open new file (yes:1 no:0)?\n";
                bool temp;
                cin>>temp;
                cin.get();
                if(temp == true) {
                    T* tempOBJ = new T();
                    cin>>*tempOBJ;
                    files.push_back(tempOBJ);
                    current = tempOBJ;
                }
                else if(files.size() > 0){
                    int index = 0;
                    files.sort();
                    for(auto it = files.begin(); it != files.end(); it++)
                        std::cout<<"\tFile: "<<index++<<endl, std::cout<<**it<<endl;
                    std::cout<<"Choose file: \n";
                    int fileNr;
                    cin>>fileNr;
                    cin.get();
                    if(fileNr >= 0 && fileNr < files.size()) {
                        for (auto it = files.begin(); it != files.end(); it++) {
                            if (fileNr == 0) {
                                current = *it;
                                break;
                            }
                            fileNr--;
                        }
                    } else cout<<"~ INVALID INDEX\n";
                }
                else cout<<"~ NO FILES\n";
                this->displayMenu();
                break;
            }
            case 2: {
                system("CLS");
                if(current != NULL) {
                    this->editEngine();
                } else cout<<"~ NO FILE SELECTED\n";
                this->displayMenu();
                break;
            }
            case 3: {
                system("CLS");
                try {
                    if (files.size() > 0) {
                        if (current == NULL) throw string("~ NO FILE SELECTED\n");
                        int index = 0;
                        files.sort();
                        for (auto it = files.begin(); it != files.end(); it++)
                            std::cout << "\tFile: " << index++ << endl, std::cout << **it << endl;
                        std::cout << "Choose file: \n";
                        int fileNr;
                        cin >> fileNr;
                        cin.get();
                        if (fileNr >= 0 && fileNr < files.size()) {
                            for (auto it = files.begin(); it != files.end(); it++) {
                                if (fileNr == 0) {
                                    current = *it;
                                    break;
                                }
                                fileNr--;
                            }
                            files.remove(current);
                            cout << "~ FILE WAS DELETED SUCCESSFULLY\n";
                        } else cout << "~ INVALID INDEX\n";

                        index = 0;
                        for (auto it = files.begin(); it != files.end(); it++)
                            std::cout << "\tFile: " << index++ << endl, std::cout << **it << endl;

                        std::cout << "Choose file: \n";
                        cin >> fileNr;
                        cin.get();

                        if (fileNr >= 0 && fileNr < files.size()) {
                            for (auto it = files.begin(); it != files.end(); it++) {
                                if (fileNr == 0) {
                                    current = *it;
                                    break;
                                }
                                fileNr--;
                            }
                        } else cout << "~ INVALID INDEX\n";
                    } else cout << "~ NO FILES\n";
                } catch(const string& err) {std::cout<<err;}
                this->displayMenu();
                break;
            }
            case 4: {
                system("CLS");
                if(current != NULL) this->displayEngine();
                else cout<<"~ NO FILE SELECTED\n";
                this->displayMenu();
                break;

            }
            case 0: {
                system("CLS");
                return;
            }
            default:
                cout << "~ INVALID OPTION\n";
        }
    }
}

template<class T>
void Project<T>::write(string output) {
    output = "../" + output;
    std::ofstream out(output);

    if(!files.empty()) {
        out << files.size() << endl << name;
        out.close();
        for (auto it = files.begin(); it != files.end(); it++) {
            out.open(output, std::ios_base::app);
            out << "\n" << (*it)->getType() << " ";
            out.close();
            (*it)->serialize(output);
        }

        std::cout << "~ EXPORT SUCCESSFUL\n";
    }
    else std::cout<<"~ NO FILES TO EXPORT\n";
}

template<>
void Project<Photoshop>::read(string input) {
    input = "../" + input;
    std::ifstream in(input);

    int nrObj;
    in>>nrObj;
    in.get();
    string nameFromFile;
    in>>nameFromFile;
    this->name = nameFromFile;

    try {
        for (int i = 0; i < nrObj; i++) {
            string cls, name, temp;
            in >> cls >> name;

            temp = cls + " " + name;

            if (temp == "class Video") throw importException;

            Photoshop *p = new Photoshop();
            if (temp == "class Effect") p->getImageByReference() = new Effect();
            if (temp == "class Adjustment") p->getImageByReference() = new Adjustment();
            if (temp == "class Edited") p->getImageByReference() = new Edited();

            p->deserialize(in);
            files.push_back(p);
        }
        std::cout << "~ IMPORT SUCCESSFUL\n";
    }
    catch (MyException& e) {cout<<e.what();}
}

template<>
void Project<Video>::read(string input) {
    input = "../" + input;
    std::ifstream in(input);

    int nrObj;
    in>>nrObj;
    in.get();

    try {
        for(int i=0;i<nrObj;i++) {
            string cls,name,temp;
            in>>cls>>name;

            temp = cls + " " + name;

            if(temp == "class Effect" || temp == "class Adjustment" || temp == "class Edited")
                throw importException;

            Video* v = new Video();
            v->deserialize(in);
            files.push_back(v);
        }
        std::cout<<"~ IMPORT SUCCESSFUL\n";
    }
    catch (MyException& e) {cout<<e.what();}
}

template<class T>
class Menu {
private:
   static Menu* singleton;
   static int nrOfInstances;
   Menu() {
       projectType = false;
       isSaved = false;
       currentProj = NULL;
   }
   Menu(const Menu&) = delete;
   // 0 for images 1 for videos
   bool projectType, isSaved;
   std::set<T*> proj;
   T* currentProj;
public:
    static Menu* getInstance() {
        nrOfInstances++;
        if(!singleton) singleton = new Menu();
        return singleton;
    }
    ~Menu() {
        nrOfInstances--;
        if(!nrOfInstances && singleton) delete singleton;
    }

    void displayProject();
    void projectEngine();
};

template<class T>
Menu<T>* Menu<T>::singleton = 0;

template<class T>
int Menu<T>::nrOfInstances = 0;

template<class T>
void Menu<T>::displayProject() {
    for (int i = 0; i < 10; i++) cout << "-";
    cout << " PROJECT PAGE ";
    for (int i = 0; i < 10; i++) cout << "-";
    cout << endl;

    cout << "1. Create new project\n";
    cout << "2. Open project\n";
    cout << "3. Save project\n";
    cout << "0. Go back\n";
}

template<class T>
void Menu<T>::projectEngine() {
    system("CLS");
    this->displayProject();

    while(true) {
        int option;
        cout << "Enter option: \n";
        cin >> option;
        cin.get();
        switch (option) {
            case 1: {
                system("CLS");
                T* temp = new T();
                cin>>*temp;
                proj.insert(temp);
                currentProj = temp;
                this->displayProject();
                break;
            }
            case 2: {
                system("CLS");
                string temp;
                std::cout<<"Enter file name: \n";
                getline(std::cin,temp);

                if(currentProj == NULL) currentProj = new T();
                currentProj->read(temp);
                proj.insert(currentProj);
                currentProj->menuEngine();
                this->displayProject();
                break;
            }
            case 3: {
                system("CLS");
                string temp;
                std::cout<<"Enter file name: \n";
                getline(std::cin,temp);

                if(currentProj != NULL) {
                    currentProj->write(temp);
                    isSaved = true;
                }

                this->displayProject();
                break;
            }
            case 0: {
                system("CLS");
                if(isSaved == 0) {
                    std::cout<<"Do you want to save changes to "<<currentProj->getName()<<" before closing (yes:1 no:0)?\n";
                    bool temp;
                    std::cin>>temp;
                    cin.get();
                    if(temp == 1) {
                        string fileName;
                        std::cout<<"Enter file name: \n";
                        getline(std::cin,fileName);
                        currentProj->write(fileName);
                    }
                }
                return;
            }
            default:
                cout << "~ INVALID OPTION\n";
        }
    }
}
// TODO if project was saved change isSaved
void displayMainMenu() {
    for (int i = 0; i < 10; i++) cout << "-";
    cout << " EDITING SOFTARE ";
    for (int i = 0; i < 10; i++) cout << "-";
    cout << endl;

    cout << "1. Edit images\n";
    cout << "2. Edit videos\n";
    cout << "0. Exit\n";
}

int main() {
    initOpenCV();

    system("CLS");
    displayMainMenu();
    while(true) {
        int option;
        cout << "Enter option: \n";
        cin >> option;
        cin.get();
        switch (option) {
            case 1: {
                system("CLS");
                Menu<Project<Photoshop>>* m = m->getInstance();
                m->projectEngine();
                displayMainMenu();
                break;
            }
            case 2: {
                system("CLS");
                Menu<Project<Video>>* m = m->getInstance();
                m->projectEngine();
                displayMainMenu();
                break;
            }
            case 0: {
                return 0;
            }
            default:
                cout << "~ INVALID OPTION\n";
        }
    }
    return 0;
}
// TODO check input type
// TODO intreaba de ce nu merge outside class operator>> la template