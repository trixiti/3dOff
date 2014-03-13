#include "Windows.h"
#define GET_ALPHA(x)		 (x & 0xFF000000) >> 24
#define GET_ROJO(x)			 (x & 0x00FF0000) >> 16
#define GET_VERDE(x)		 (x & 0x0000FF00) >> 8
#define GET_AZUL(x)			 (x & 0x000000FF)
#define MAKE_PIXEL(a,r,g,b) ((a & 0x000000FF) << 24) + ((r & 0x000000FF) << 16) + ((g & 0x000000FF) << 8) + (b & 0x000000FF)

Windows::Windows(QWidget *parent) : QGLWidget(QGLFormat(QGL::SampleBuffers), parent)
{
	maximoValorHistograma=0;
	tfUI=new TransferFunction(this);
    tfUI->setWindowFlags(Qt::Tool);
	setMinimumSize(SCREEN_WIDTH, SCREEN_HEIGHT);
	histo=-1;
	clickLeft=clickRight=false;
    showMaximized();
    //tfUI->setGeometry(width()-525,height()-355,tfUI->width(),tfUI->height());
    tfUI->show();
}

Windows::~Windows(void)
{
}
GLuint Windows::loadTexture2D(char * imgName)	
{
	GLuint textureID;
	QImage image(imgName);
	image=QGLWidget::convertToGLFormat(image);
	glGenTextures( 1, &textureID );
	glBindTexture( GL_TEXTURE_2D, textureID );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width(), image.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, image.bits());
	return textureID;
}

GLuint Windows::loadTexture3D(char * imgName,bool RAW,int widthImagen,int heightImagen,int depthImagen)	
{
	GLuint textureID;
	if(RAW)  v.loadRAW(imgName,widthImagen,heightImagen,depthImagen);
	else     v.load(imgName);//"Models/Porsche.pvm");
	glGenTextures(1,&textureID);
	glBindTexture(GL_TEXTURE_3D,textureID);
	glPixelStorei(GL_UNPACK_ALIGNMENT,1);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP); 
	glTexImage3D(GL_TEXTURE_3D,0,GL_LUMINANCE,v.width,v.height,v.depth,0,GL_LUMINANCE,GL_UNSIGNED_BYTE,v.volume);
	glBindTexture(GL_TEXTURE_3D,0);
	return textureID;
}


void Windows::initializeGL()
{
	// Inicializar Tiempo para FPS
	m_time=QTime::currentTime();
	sec=0;
	fps=0;


	glewInit();
	if (glewIsSupported("GL_VERSION_2_0"))
		printf("Ready for OpenGL 2.0\n");
	else 
	{
		printf("OpenGL 2.0 not supported\n");
		close();
	}

	shaders.push_back(Shader("vertexColor.glsl","fragmentColor.glsl"));
	shaders.push_back(Shader("vertexRayCasting.glsl","fragmentRayCasting.glsl"));
	glGenTextures(1, &histo);
	texid=-1;
	//texid=loadTexture3D("Models/Porsche.pvm");
	//texid=loadTexture3D("Models/Teddy.pvm");
	//texid=loadTexture3D("Models/Carp.pvm");
	//texid=loadTexture3D("Models/Box.pvm");
	//texid=loadTexture3D("Models/Engine.pvm");
	//texid=loadTexture3D("Models/Bunny.pvm");
	//texid=loadTexture3D("Models/MRI-Woman.pvm");
	//calcularHistograma();
	m_timer = new QTimer(this);
	m_timer->setInterval(1);
	connect(m_timer, SIGNAL(timeout()), this, SLOT(updateGL()));
	m_timer->start();
	scaleX=scaleY=scaleZ=1;rotateX=rotateY=rotateZ=0;
	rotate[0]=scale[0]=true;
	rotate[1]=rotate[2]=scale[1]=scale[2]=false;

}
void Windows::calcularHistograma()
{
	printf("Calculando Histograma... ");
	memset(histograma,0,sizeof(histograma));
	for(int i=0;i<v.depth*v.height*v.width;++i) ++histograma[v.volume[i]];
	maximoValorHistograma=0;
	for(int i=1;i<255;++i){
		if(maximoValorHistograma<histograma[i]) maximoValorHistograma=histograma[i];
	}
	scaleX=v.scaley;
	scaleY=v.scalex;
	scaleZ=v.scalez;
	tfUI->ui.DisplayR->update();
	tfUI->ui.DisplayG->update();
	tfUI->ui.DisplayB->update();
	tfUI->ui.DisplayA->update();
	printf("Fin del calculo...\n");
}
inline void Windows::Cube(float bug){
	float texDec=bug;
	float texInc=1.0f-texDec;
	float CorDec=texDec*20.0f-10.0f;
	float CorInc=texInc*20.0f-10.0f;
	glBegin(GL_QUADS);
		glVertex3f( CorInc, CorDec, CorDec );
		glVertex3f( CorDec, CorDec, CorDec );
		glVertex3f( CorDec, CorInc, CorDec );
		glVertex3f( CorInc, CorInc, CorDec );
		glVertex3f( CorInc, CorDec, CorInc );
		glVertex3f( CorInc, CorDec, CorDec );
		glVertex3f( CorInc, CorInc, CorDec );
		glVertex3f( CorInc, CorInc, CorInc );
		glVertex3f( CorDec, CorDec, CorInc );
		glVertex3f( CorInc, CorDec, CorInc );
		glVertex3f( CorInc, CorInc, CorInc );
		glVertex3f( CorDec, CorInc, CorInc );
		glVertex3f( CorDec, CorDec, CorDec );
		glVertex3f( CorDec, CorDec, CorInc );
		glVertex3f( CorDec, CorInc, CorInc );
		glVertex3f( CorDec, CorInc, CorDec );
		glVertex3f( CorDec, CorInc, CorDec );
		glVertex3f( CorDec, CorInc, CorInc );
		glVertex3f( CorInc, CorInc, CorInc );
		glVertex3f( CorInc, CorInc, CorDec );
		glVertex3f( CorDec, CorDec, CorDec );
		glVertex3f( CorInc, CorDec, CorDec );
		glVertex3f( CorInc, CorDec, CorInc );
		glVertex3f( CorDec, CorDec, CorInc );
	glEnd();
}
void Windows::paintGL()
{ 	
	//OpenGL Initialize
	GLint oldDrawbuf = 0;
	glMatrixMode(GL_MODELVIEW);
	glClearColor(0.0f, 0.0f,0.0f, 1.0); 
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glGetIntegerv(GL_DRAW_BUFFER, &oldDrawbuf);
	glEnable(GL_DEPTH_TEST);
	glLoadIdentity();

	
	glTranslated(0.0f,0.0f,-40.0f);

	if(texid==-1) return;

	glScalef(scaleX,scaleY,scaleZ);
	glRotated(rotateX,1,0,0);
	glRotated(rotateY,0,1,0);
	glRotated(rotateZ,0,0,1);

	glDisable(GL_LIGHTING);
	shaders[0].applyShader();
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);
	glEnable(GL_CULL_FACE);
	glViewport(0,0,bufferWi,bufferHe);

	//First Pass
	{
		glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
		glClearColor(0.0, 0.0, 0.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glCullFace(GL_BACK);
		Cube(0.0f);
	}

	//Second Pass
	{
		glDrawBuffer(GL_COLOR_ATTACHMENT1_EXT);
		glClearColor(0.0, 0.0, 0.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glCullFace(GL_FRONT);
		Cube(0.0f);
	}

	shaders[0].unApplyShader();
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	glDrawBuffer(oldDrawbuf);
	glViewport(0,0,wi,he);

	//Ray-Casting Pass
	{
		shaders[1].applyShader();

		glClearColor(0.0, 0.0, 0.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glCullFace(GL_BACK);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, buffer0); //Front face

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, buffer1); //Back face

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_3D, texid);   //Volume
		
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_1D, histo);   //Transfer Function 

		//Check changes in Transfer Function 
		if(tfUI->ui.DisplayA->change || tfUI->ui.DisplayR->change || tfUI->ui.DisplayG->change || tfUI->ui.DisplayB->change){
			const float ajuste=255.0f/150.0f;
			unsigned int arr[256];
			for(int j=0;j<256;++j){
				unsigned int A=(unsigned int)(float(tfUI->ui.DisplayA->trans[j])*ajuste+0.5f);
				unsigned int R=(unsigned int)(float(tfUI->ui.DisplayR->trans[j])*ajuste+0.5f);
				unsigned int G=(unsigned int)(float(tfUI->ui.DisplayG->trans[j])*ajuste+0.5f);
				unsigned int B=(unsigned int)(float(tfUI->ui.DisplayB->trans[j])*ajuste+0.5f);
				arr[j]=MAKE_PIXEL(A,B,G,R);
			}
			tfUI->ui.DisplayA->change = tfUI->ui.DisplayR->change = tfUI->ui.DisplayG->change = tfUI->ui.DisplayB->change = false;
			glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_WRAP_S,     GL_REPEAT);
			glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
			glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
			glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, arr);
		}
		glActiveTexture(GL_TEXTURE0);

		shaders[1].setInt("frontBuffer",0);
		shaders[1].setInt("backBuffer",1);
		shaders[1].setInt("volume",2);
		shaders[1].setInt("transferFunction",3);
		shaders[1].setInt("width",wi);
		shaders[1].setInt("height",he);
		Cube(0.01f);

		shaders[1].unApplyShader();
	}

	
	
	//FPS counter
	++fps;
	const int gamesT=GAMETIME;
	const int secAct=gamesT/1000;
	if(secAct!=sec && fps>0){
		ratio=double(fps)/double(secAct-sec);
		sec=secAct;
		setWindowTitle(QString("FPS: ")+QString::number(ratio));
		fps=0;
	}

	// Display Debug Text
	glColor3f(1.0f,1.0f,1.0f);
	debugDisplay=QString("FPS: ")+QString::number(ratio)+QString(" Time: ")+QString::number(gamesT);
	renderText(10,10,debugDisplay);
}

void Windows::resizeGL(int width, int height)
{
	wi=width;
	he=height;
	bufferWi=wi*2;
	bufferHe=he*2;

	glViewport(0,0,width,height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//gluOrtho2D(-10,10,-10,10);
	gluPerspective(45.0f, (float)width/(float)height, 0.1f, 500.0f);

	//Delete resources
	glDeleteTextures(1, &buffer0);
	glDeleteTextures(1, &buffer1);
	glDeleteRenderbuffersEXT(1, &depth_rb);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	glDeleteFramebuffersEXT(1, &fb);

	glGenFramebuffersEXT(1, &fb);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);
	glGenRenderbuffersEXT(1, &depth_rb);
	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, depth_rb);
	glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, bufferWi, bufferHe);
	glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, depth_rb);

	glGenTextures(1, &buffer0);
	glBindTexture(GL_TEXTURE_2D, buffer0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, bufferWi, bufferHe, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);

	glGenTextures(1, &buffer1);
	glBindTexture(GL_TEXTURE_2D, buffer1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, bufferWi, bufferHe, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);

	//Attach 2D texture to this FBO
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, buffer0, 0);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_TEXTURE_2D, buffer1, 0);
	GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	switch(status)
	{
	 case GL_FRAMEBUFFER_COMPLETE_EXT: 
		 printf("FrameBuffer: OK\n");
		 break;
	default:
		printf("FrameBuffer: Error\n");
	}
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

}

void Windows::mouseMoveEvent(QMouseEvent *event)
{
	if(clickLeft || clickRight){
		int dx=QCursor::pos().x()-mX;
		int dy=QCursor::pos().y()-mY;
		if(clickRight){
			if(scale[0]) scaleX+=float(dx)*0.01f;
			if(scale[1]) scaleY+=float(dy)*0.01f;
			if(scale[2]) scaleZ+=float(dy)*0.01f;
		}
		if(clickLeft){
			
			if(rotate[1]) rotateX+=float(dy)*2.0f;
			if(rotate[0]) rotateY+=float(dx)*2.0f;
			if(rotate[2]) rotateZ+=float(dy)*2.0f;
		}
		QWidget *p= parentWidget()->parentWidget();
		QCursor::setPos(width()/2 + geometry().left() + p->geometry().left(), height()/2 + geometry().top() + p->geometry().top());
		mX=QCursor::pos().x();
		mY=QCursor::pos().y();
	}

}

void Windows::mousePressEvent(QMouseEvent *event)
{	
	QWidget *p= parentWidget()->parentWidget();
	QCursor::setPos(width()/2 + geometry().left() + p->geometry().left(), height()/2 + geometry().top() + p->geometry().top());
	mX=QCursor::pos().x();
	mY=QCursor::pos().y();
	if(event->button()==Qt::LeftButton){
		clickLeft=true;
	}
	if(event->button()==Qt::RightButton){
		clickRight=true;
	}
}
void Windows::mouseReleaseEvent(QMouseEvent *event)
{	
	if(event->button()==Qt::LeftButton)
		clickLeft=false;

	if(event->button()==Qt::RightButton)
		clickRight=false;
}

void Windows::keyPressEvent(QKeyEvent *event)
{
	if(event->key()==Qt::Key_Escape){
		close();
	}

	if(event->key()==Qt::Key_Escape){
		close();
	}


	if(event->key()==Qt::Key_R){
		shaders[1]=Shader("vertexRayCasting.glsl","fragmentRayCasting.glsl");
	}

	if(event->key()==Qt::Key_0)
	{
		if(this->isFullScreen())
		{
			showNormal();
		}
		else
		{
			showFullScreen();
		}
	}

	float dLR=0.0f, dUD=0.0f, dEL = 0.0f;

}


void Windows::keyReleaseEvent(QKeyEvent *event)
{

}
