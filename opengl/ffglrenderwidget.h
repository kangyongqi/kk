#ifndef FFGLRENDERWIDGET_H
#define FFGLRENDERWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLShaderProgram>
#include <QVector>
#include <QOpenGLTexture>
#include <QOpenGLFunctions_4_5_Core>
#include<iostream>

extern "C"{
#include<libavformat/avformat.h>
}

class FFGLRenderWidget : public QOpenGLWidget, protected QOpenGLFunctions_4_5_Core
{
    Q_OBJECT

signals:
    void mouseDoubleClick();
    void mouseClick();
public:
    explicit FFGLRenderWidget(QWidget *parent = nullptr);
    ~FFGLRenderWidget() override;
    void setAspect(float aspect_);
    void setBlackScreen();
    void setKeepRatio(bool flag);
public slots:
//    void setYUVData(uint8_t *yData, uint8_t *uData, uint8_t *vData, int width, int height);
    void setYUVData(AVFrame* frame);
protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;
    void mouseDoubleClickEvent(QMouseEvent*event)override;
    void mousePressEvent(QMouseEvent*event)override;
    void initShaders();
private:
    static const QVector<float> vertices;
    static const QVector<unsigned int> indices;

    GLuint vao = 0, vboVertice = 0, ebo = 0;
    GLuint yTexture;
    GLuint uTexture;
    GLuint vTexture;
    QOpenGLShaderProgram* shaderProgram = nullptr;

    // 视口参数
    float aspect = 16.0f /9;
    int viewportX = 0;
    int viewportY = 0;
    int viewportWidth = 0;
    int viewportHeight = 0;

    bool keepRatio = false;
};

#endif // FFGLRENDERWIDGET_H
