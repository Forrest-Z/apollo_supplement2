#pragma once
 
#include <Eigen/Geometry>

namespace pose_align {

    class Transform {
    public:
        EIGEN_MAKE_ALIGNED_OPERATOR_NEW

        typedef Eigen::Matrix<double, 6, 1> Vector6;
        typedef Eigen::Quaterniond Rotation;
        typedef Eigen::Vector3d Translation;
        typedef Eigen::Matrix<double, 4, 4> Matrix;

        Transform() {
            rotation_.setIdentity();
            translation_.setZero();
        }

        Transform(const Translation &translation, const Rotation &rotation)
                : translation_(translation), rotation_(rotation) {}

        const Rotation &rotation() const { return rotation_; }

        const Translation &translation() const { return translation_; }

        Matrix matrix() const {
            Matrix matrix;
            matrix.setIdentity();
            matrix.topLeftCorner<3, 3>() = rotation_.matrix();
            matrix.topRightCorner<3, 1>() = translation_;
            return matrix;
        }

        Transform inverse() const {
            const Rotation rotation_inverted(rotation_.w(), -rotation_.x(),
                                             -rotation_.y(), -rotation_.z());
            return Transform(-(rotation_inverted * translation_), rotation_inverted);
        }

        Transform operator*(const Transform &rhs) const {
            return Transform(translation_ + rotation_ * rhs.translation(),
                             rotation_ * rhs.rotation());
        }

        static Transform exp(const Vector6 &vector) {
            constexpr float kEpsilon = 1e-8;
            const float norm = vector.tail<3>().norm();
            if (norm < kEpsilon) {
                return Transform(vector.head<3>(), Rotation::Identity());
            } else {
                return Transform(vector.head<3>(), Rotation(Eigen::AngleAxisd(
                        norm, vector.tail<3>() / norm)));
            }
        }

        Vector6 log() const {
            Eigen::AngleAxisd angle_axis(rotation_);
            return (Vector6() << translation_, angle_axis.angle() * angle_axis.axis())
                    .finished();
        }

        Rotation rotation_;
        Translation translation_;
    };
}  // namespace pose_align
