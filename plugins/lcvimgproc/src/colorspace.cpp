#include "colorspace.h"
#include "cvextras.h"
#include "live/viewcontext.h"
#include "live/visuallogqt.h"

ColorSpace::ColorSpace(QObject *parent)
    : QObject(parent)
{

}

QMat *ColorSpace::cvtColor(QMat *input, int code, int dstCn)
{
    if (!input || input->internal().empty()) return nullptr;
    try{
        cv::Mat* rMat = new cv::Mat(
            input->dimensions().height(),
            input->dimensions().width(),
            CV_MAKETYPE(input->depth(), input->channels()));
        cv::cvtColor(input->internal(), *rMat, code, dstCn);
        return new QMat(rMat);

    } catch (cv::Exception& e){
        lv::CvExtras::toLocalError(e, engine(), this, "ColorSpace: ").jsThrow();
        return nullptr;
    }
}

lv::ViewEngine *ColorSpace::engine(){
    lv::ViewEngine* ve = lv::ViewEngine::grabFromQmlEngine(qobject_cast<QQmlEngine*>(parent()));
    if ( !ve )
        lv::QmlError::warnNoEngineCaptured(this, "ColorSpace");
    return ve;
}
