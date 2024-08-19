import * as THREE from 'three';

export function Move(id, anchorDir, deltaPos, sliding = false) {
    this.id = id;
    this.anchorDir = anchorDir;
    this.deltaPos = deltaPos;
    this.sliding = sliding;

    this.maxAngle = 0;
    this.preTrans = 0;
    this.rotAxis = 0;

    if (!sliding) {
        let _minPreTrans = new THREE.Vector3(-0.5, -0.5, -0.5);
        let _maxPreTrans = new THREE.Vector3(0.5, 0.5, 0.5);
        this.preTrans = anchorDir.clone().add(deltaPos).multiplyScalar(0.5).clamp(_minPreTrans, _maxPreTrans);
        this.postTrans = this.preTrans.clone().negate();

        this.rotAxis = anchorDir.clone().cross(deltaPos).normalize();

        this.maxAngle = THREE.MathUtils.degToRad(deltaPos.toArray().reduce((p,a)=>p+Math.abs(a), 0.0) * 90.0);
    }
}
