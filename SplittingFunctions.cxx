{
  // TF1* Pqq = new TF1("Pqq","4./3.* (1+x*x)")
  TF1* Pgq = new TF1("Pgq","4./3.* (1+pow((1-x),2)) / x", 0,1);
  Pgq->Draw();

}
