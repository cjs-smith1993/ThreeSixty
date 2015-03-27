var mongoose = require('mongoose');
var postSchema = new mongoose.Schema({
	title: String,
	upvotes: {
		type: Number,
		default: 0
	},
	comments: [{
		type: mongoose.Schema.Types.ObjectId,
		ref: 'Comment'
	}]
});

postSchema.methods.upvote = function(cb) {
	this.upvotes++;
	this.save(cb);
}

mongoose.model('Post', postSchema);