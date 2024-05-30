#ifndef PLOT_HPP__
#define PLOT_HPP__

#include <math.h>
#include <stdio.h>
#include <thread>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <vector>

#include "ring_vector.hpp"

#define PLOTTER_COLOR_RED (cv::Scalar(0, 0, 255))
#define PLOTTER_COLOR_WHITE (cv::Scalar(255, 255, 255))
#define PLOTTER_COLOR_BLUE (cv::Scalar(0, 255, 0))
#define PLOTTER_COLOR_GREEN (cv::Scalar(255, 0, 0))
#define PLOTTER_COLOR_BLACK (cv::Scalar(0, 0, 0))
#define PLOTTER_COLOR_GRAY (cv::Scalar(230, 230, 230))

// 後輩へ. あまりにも汚いコードですけど，でも使い捨てコードなのでリファクタリングはしません．
namespace Serial {

typedef struct vec2D {
    double x;
    double y;
} Point2D;

typedef struct rect2D {
    Point2D origin;
    Point2D size;
    Point2D end;
} Frame2D;

typedef struct figurePos {
    int xdiv;
    int ydiv;
    int pos;
} FigurePos;

typedef struct range2D {
    double min;
    double max;
} Range2D;

typedef struct plot {
    RingVector<Point2D> *buf;

    Range2D xrange;
    Range2D yrange;

    std::string xlabel;
    std::string ylabel;

    double xtics;
    double ytics;

    FigurePos pos;

    int xtics_digit;
    int ytics_digit;
} Plot;

class Plotter {
  private:
    const double margin_ = 25.0;
    // ほんとはPlotクラスを作って, その中で自分の範囲だけを描画できるようにしたい
    // そうすると, あとあと, 折れ線グラフ以外の実装も簡単にできるようになる．
    // けど時間ないので, 今回はこれで．というかこんなに機能いらない．
    std::vector<Plot> store_;
    Point2D plotSize_;
    cv::Mat canvas_;
    bool procCancel_ = false;

    Point2D calcPosition(FigurePos pos) {
        double x = (pos.pos - 1) % pos.xdiv + 1;
        double y = (pos.pos - 1) / pos.xdiv + 1;
        return {x, y};
    }

    Frame2D calcBounds(FigurePos pos, double margin) {
        Point2D fig_pos = calcPosition(pos);
        double each_w = plotSize_.x / (double)pos.xdiv;
        double each_h = plotSize_.y / (double)pos.ydiv;
        int pivx = fig_pos.x - 1;
        int pivy = fig_pos.y - 1;
        double originx = (double)pivx * each_w + margin;
        double originy = (double)pivy * each_h + margin;
        return {{originx, originy},
                {each_w, each_h},
                {originx + each_w, originy + each_h}};
    }

    Frame2D calcFrame(FigurePos pos, double margin) {
        double offsetx = margin;
        double offsety = margin;
        Frame2D frame = calcBounds(pos, margin);
        double each_w_margin = frame.size.x - (margin * 2.0);
        double each_h_margin = frame.size.y - (margin * 2.0);
        return {{frame.origin.x + margin, frame.origin.y + margin},
                {each_w_margin, each_h_margin},
                {frame.origin.x + margin + each_w_margin,
                 frame.origin.y + margin + each_h_margin}};
    }

    inline double inGraphX(Frame2D frame, double pos) {
        return frame.origin.x + pos;
    }

    inline double inGraphY(Frame2D frame, double pos) {
        return frame.origin.y + pos;
    }

    inline cv::Point inGraph(double x, double y, Frame2D frame) {
        return cv::Point(inGraphX(frame, x), inGraphY(frame, y));
    }

    inline double inDrawX(Plot info, Frame2D frame, double x){
        double dx = frame.size.x / fabs(info.xrange.max - info.xrange.min);
        // std::cout << dx << std::endl;
        return inGraphX(frame, (dx * x) - (info.xrange.min * dx));
    }

    inline double inDrawY(Plot info, Frame2D frame, double y){
        double dy = frame.size.y / fabs(info.yrange.max - info.yrange.min);
        return inGraphY(frame, (dy * y) - (info.yrange.min * dy));
    }

    inline cv::Point inDraw(double x, double y, Plot info, Frame2D frame){
        double qx = inDrawX(info, frame, x);
        double qy = inDrawY(info, frame, y);
        return cv::Point(qx, qy);
    }

    inline double tics(int i) {
        if (i % 5 == 0) {
            return 6;
        } else if (i % 10 == 0) {
            return 10;
        }

        return 4;
    }

    void drawTics(Plot info) {
        Frame2D bounds = calcBounds(info.pos, margin_);
        Frame2D frame = calcFrame(info.pos, margin_);       

        // x軸の数の計算
        double nx =
            floor(fabs(info.xrange.max - info.xrange.min) / (double)info.xtics);
        double ny =
            floor(fabs(info.yrange.max - info.yrange.min) / (double)info.ytics);

        double dx = frame.size.x / fabs(info.xrange.max - info.xrange.min) *
                    (double)info.xtics;
        double dy = frame.size.y / fabs(info.yrange.max - info.yrange.min) *
                    (double)info.ytics;

        // グリッドの描画
        for (int i = 0; i < (int)nx + 1; i++) {
            cv::line(canvas_, inGraph(dx * i, 0, frame),
                     inGraph(dx * i, frame.size.y, frame), PLOTTER_COLOR_GRAY,
                     1, 4);
        }

        for (int i = (int)ny; i > 0; i--) {
            cv::line(canvas_, inGraph(0, dy * i, frame),
                     inGraph(frame.size.x, dy * i, frame), PLOTTER_COLOR_GRAY,
                     1, 4);
        }

        // tixsの描画
        for (int i = 0; i < (int)nx + 1; i++) {
            cv::line(canvas_, inGraph(dx * i, frame.size.y, frame),
                     inGraph(dx * i, frame.size.y - tics(i), frame),
                     PLOTTER_COLOR_BLACK, 1, 4);

            // tics文字列
            std::ostringstream oss;
            oss << std::fixed << std::setprecision(info.xtics_digit)
                << info.xrange.min + (double)info.xtics * i;
            std::string ticsX = oss.str();

            // tics文字列の描画
            int baseline = 0;
            double thickness = 0.5;
            cv::Size textSize = cv::getTextSize(ticsX, cv::FONT_HERSHEY_SIMPLEX,
                                                0.5, thickness, &baseline);
            baseline += thickness;
            cv::Point point =
                (i != 0) ? inGraph(dx * i - textSize.width / 2.0,
                                   frame.size.y + textSize.height + 6.0, frame)
                         : inGraph(dx * i, frame.size.y + textSize.height + 6.0,
                                   frame);
            cv::putText(canvas_, ticsX, point, cv::FONT_HERSHEY_SIMPLEX, 0.5,
                        cv::Scalar(0, 0, 0), thickness);
        }

        for (int i = (int)ny; i >= 0; i--) {
            cv::line(canvas_, inGraph(0, dy * i, frame),
                     inGraph(tics(i), dy * i, frame), PLOTTER_COLOR_BLACK, 1,
                     4);

            // tics文字列
            std::ostringstream oss;
            oss << std::fixed << std::setprecision(info.ytics_digit)
                << info.yrange.min + (double)info.ytics * i;
            std::string ticsY = oss.str();

            // tics文字列の描画
            int baseline = 0;
            double thickness = 0.5;
            cv::Size textSize = cv::getTextSize(ticsY, cv::FONT_HERSHEY_SIMPLEX,
                                                0.5, thickness, &baseline);
            baseline += thickness;
            cv::Point point = inGraph(-textSize.width - 6.0,
                                      dy * i + textSize.height / 2.0, frame);
            cv::putText(canvas_, ticsY, point, cv::FONT_HERSHEY_SIMPLEX, 0.5,
                        cv::Scalar(0, 0, 0), thickness);
        }

        cv::rectangle(canvas_, inGraph(0, 0, frame),
                      inGraph(frame.size.x, frame.size.y, frame),
                      PLOTTER_COLOR_BLACK, 1);
    }

    bool isEqualPos(FigurePos lhs, FigurePos rhs) {
        return (lhs.xdiv == rhs.xdiv) && (lhs.ydiv == rhs.ydiv) &&
               (lhs.pos == rhs.pos);
    }

    void drawGraphs(Plot info){     
        Frame2D frame = calcFrame(info.pos, margin_);
            
        for (int i = 0; i < info.buf->getDataLength() - 1; i++) {
            Point2D sp = info.buf->readl(i);
            Point2D ep = info.buf->readl(i + 1);

            if (sp.x > info.xrange.max || sp.x < info.xrange.min){
                continue;
            }

            if (sp.y > info.yrange.max || sp.y < info.yrange.min){
                continue;
            }

            cv::Point cvsp = inDraw(sp.x, sp.y, info, frame);
            cv::Point cvep = inDraw(ep.x, ep.y, info, frame);
            
            cv::line(canvas_, cvsp, cvep, PLOTTER_COLOR_RED, 1, cv::LINE_AA);                      
        }
    }

  public:
    Plotter() {}
    ~Plotter() {}

    void figure(int width = 1080, int height = 680) {
        canvas_ = cv::Mat(cv::Size(width + margin_ * 2, height + margin_ * 2),
                          CV_8UC3, PLOTTER_COLOR_WHITE);
        plotSize_ = {(double)width, (double)height};
    }

    void addSubplot(FigurePos pos, int max_width_sample = 4096) {
        RingVector<Point2D> *buf =
            new RingVector<Point2D>(max_width_sample, {0.0, 0.0});
        Range2D xrange = {0.0, 20.0};
        Range2D yrange = {-1.0, 1.0};

        Plot info = {buf,   xrange, yrange, "xlabel", "ylabel",
                     200.0, 0.5,    pos,    0,        1};
        store_.push_back(info);
    }

    void xlabel(std::string label, FigurePos pos) {
        for (auto &it : store_) {
            if (isEqualPos(it.pos, pos)) {
                it.xlabel = label;
                return;
            }
        }
    }

    void xrange(Range2D range, FigurePos pos) {
        for (auto &it : store_) {
            if (isEqualPos(it.pos, pos)) {
                it.xrange = range;
                return;
            }
        }
    }

    void ylabel(std::string label, FigurePos pos) {
        for (auto &it : store_) {
            if (isEqualPos(it.pos, pos)) {
                it.ylabel = label;
                return;
            }
        }
    }

    void yrange(Range2D range, FigurePos pos) {
        for (auto &it : store_) {
            if (isEqualPos(it.pos, pos)) {
                it.yrange = range;
                return;
            }
        }
    }

    void xdeci(int num, FigurePos pos){
        for (auto &it : store_) {
            if (isEqualPos(it.pos, pos)) {
                it.xtics_digit = num;
                return;
            }
        }
    }

     void ydeci(int num, FigurePos pos){
        for (auto &it : store_) {
            if (isEqualPos(it.pos, pos)) {
                it.ytics_digit = num;
                return;
            }
        }
    }

    void xtics(double tics, FigurePos pos) {
        for (auto &it : store_) {
            if (isEqualPos(it.pos, pos)) {
                it.xtics = tics;
                return;
            }
        }
    }

    void ytics(double tics, FigurePos pos) {
        for (auto &it : store_) {
            if (isEqualPos(it.pos, pos)) {
                it.ytics = tics;
                return;
            }
        }
    }

    void plot() {
      for (std::vector<Plot>::const_iterator it = store_.begin(),
                                               e = store_.end();
             it != e; ++it) {
            drawGraphs(*it);
        }
    }

    // TODO: 実装する
    // void plot(const double *x, const double *y, FigurePos pos) {}
    void addData(FigurePos pos, double x, double y) {
        for (auto &it : store_) {
            if (isEqualPos(it.pos, pos)) {
                if(it.buf->getDataLength() >= it.buf->getBufferCapacity()){
                    it.buf->popl();
                }

                it.buf->push({x, y});
                return;
            }
        }
    }

    void redrawFrame() {
        for (std::vector<Plot>::const_iterator it = store_.begin(),
                                               e = store_.end();
             it != e; ++it) {
            drawTics(*it);
        }
    }

    int draw(int wait) {
        // Fill
        cv::rectangle(canvas_, cv::Point(0, 0),
                      cv::Point(plotSize_.x, plotSize_.y),
                      cv::Scalar(255, 255, 255), cv::FILLED);
        // Frame
        redrawFrame();
        plot();              

        cv::imshow("Plotter", canvas_);
        return cv::waitKey(wait);
    }  
};
} // namespace Serial

#endif